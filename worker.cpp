#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h>

#include "worker.hpp"

// Hastily attempt to reduce number of memory leaks using this function
static void cleanup(int m, int r)
{
  char* line = (char*)malloc(MAXLINE*sizeof(char));
  // Free every pointer to an output of the map function
  for (int i = 0; i < m; i++) {
    char* partpath = (char*)malloc(50*sizeof(char));
    sprintf(partpath, "./program/intermediate%d", i);
    FILE* filepart = fopen(partpath, "r");
    while (fgets(line, MAXLINE, filepart) != NULL) {
      void* addr1;
      void* addr2;
      sscanf(line, "%p %p", &addr1, &addr2);
      free(addr1);
      free(addr2);
    }
    fclose(filepart);
    free(partpath);
  }
  // Free all the pointers to the reduce function's output
  for (int i = 0; i < r; i++) {
    char* outpath = (char*)malloc(50*sizeof(char));;
    sprintf(outpath, "./program/out%d", i);
    FILE* out = fopen(outpath, "r");
    while (fgets(line, MAXLINE, out) != NULL) {
      void* addr1;
      void* addr2;
      sscanf(line, "%p %p", &addr1, &addr2);
      free(addr1);
      free(addr2);
    }
    fclose(out);
    free(outpath);
  }
  free(line);
}

static void set_output_file(char* path, struct pair * pair_list, int length)
{
  FILE * wptr;
  wptr = fopen(path, "w");
  for (int i = 0; i < length; i++) {
    fprintf(wptr, "%p %p\n", pair_list[i].key, pair_list[i].value);
  }
  fclose(wptr);
}

void aggregate_outputs(FILE* final, char* path_base, int max_name)
{
  char line[BUFSIZE];
  for (int i = 0; i < max_name; i++) {
    char s_name[10];
    sprintf(s_name, "%d", i);
    char path [strlen(path_base) + strlen(s_name) + 1];
    strcpy(path, path_base);
    strcat(path, s_name);
    FILE* fptr = fopen(path, "r");
    // Modified from https://stackoverflow.com/questions/11384032/merge-n-files-using-a-c-program/11384194
    rewind(fptr);
    int write_sz;
    while ((write_sz = fread(line, sizeof(char), BUFSIZE, fptr))) {
      fwrite(line, sizeof(char), write_sz, final);
    }
    fclose(fptr);
  }
  fclose(final);
}

static int alphcmp(const void* ptr1, const void* ptr2)
{
  const char** str1 = (const char**) ptr1;
  const char** str2 = (const char**) ptr2;
  return (strcmp(*str1, *str2));
}

int sort_file(char* finalpath, char* path)
{
  FILE* input = fopen(path, "r");
  int length = 0;
  char line[MAXLINE];
  while (fgets(line, sizeof(line), input)) {
    length++;
  }
  rewind(input);
  char* list[length];
  for (int i = 0; i < length; i++) {
    list[i] = (char*)malloc(sizeof(char)*MAXLINE);
    fgets(list[i], BUFSIZE, input);
  }
  qsort(list, length, sizeof (char*), alphcmp);
  FILE* sorted = fopen(finalpath, "w");
  for (int i = 0; i < length; i++) {
    fprintf(sorted, "%s", list[i]);
  }
  fclose(sorted);
  fclose(input);
  return length;
}

static void get_output_file_portion(FILE* fp, struct pair* pair_list, int m, int n)
{
  char line[MAXLINE];
  int i = 0;
  int j = 0;
  rewind(fp);
  while (fgets(line, MAXLINE, fp) != NULL) {
    if ((i >= m) && (i < n)) {
      struct pair* ret = (struct pair*)malloc(sizeof(struct pair));
      /* ret->key = malloc(sizeof(void*)); */
      void* addr1 = 0x0;
      void* addr2 = 0x0;
      sscanf(line, "%p %p", &addr1, &addr2);
      //printf("%p, %p are read values\n", addr1, addr2);
      ret->key = addr1;
      /* printf("%i vs %i\n", ret->key, &addr1); */
      ret->value = addr2;
      pair_list[j] = *ret;
      j++;
    }
    i++;
  }
}

static void retrieve_correct_portion(long piece, long total, char* sorted_path, struct pair** input, long length) {
  struct pair * pair_list = *input;
  FILE* fptr = fopen(sorted_path, "r");
  char* prevline = (char*)malloc(MAXLINE * sizeof(char));
  char* line = (char*)malloc(MAXLINE * sizeof(char));
  int i = 0;
  int range[2] = {length, 0};
  while (fgets(line, sizeof(line), fptr)) {
    char* key  = (char*)malloc(MAXLINE * sizeof(char));
    sscanf(line, "%s *s", key);
    strcpy(line, key);
    if ((i >= (piece*length)/total)  && (strcmp(line, prevline)!=0)) {
      if (i <= ((1+piece)*length)/total) {
        if (i < range[0]) range[0] = i;
        else if (i > range[1]) range[1] = i;
      }
      else {
        if (i < range[0]) range[0] = i;
        else if (i > range[1]) range[1] = i;
        break;
      }
    }
    strcpy(prevline, line);
    i++;
    free(key);
  }
  if ((i >= (piece*length)/total) && (i <= ((1+piece)*length)/total)) {
    if (i > range[1]) range[1] = i;
  }
  //printf("PORTION IS %i to %i\n", range[0],  range[1]);
  get_output_file_portion(fptr, pair_list, range[0], range[1]);
  pair_list[range[1]-range[0]].key = 0x0;
  pair_list[range[1]-range[0]].value = 0x0;
}

// This tutorial helped quite a bit in debugging what was going wrong with connection
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
void* start_worker(void* arguments)
{
  struct args *function_args = (struct args *)arguments;
  char* start = "Online.";

  // Same socket is needed on client end so initialize all over again.
  int s;
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("\n Error : Socket Failed \n");
  }
  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; // Specify address family.
  addr.sin_addr.s_addr = htonl(function_args->ip); // INADDR_ANY just 0.0.0.0, machine IP address
  addr.sin_port = htons(PORT); // Specify port.

  // Connect to server
  if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("\n Error : Connect Failed \n");
  }
  // NOTE This can and will not work if flag argument set to 1
  sendto(s, start, BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
  printf("Worker%i │ Informed server of existence.\n", function_args->name);
  char buf[BUFSIZE];
  int recvlen;
  socklen_t len = sizeof(addr);
  struct pair* results = (struct pair*)malloc(sizeof(struct pair)*function_args->length);
  struct pair* out = (struct pair*)malloc(sizeof(struct pair)*(function_args->length+1));
  while (1==1) {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &len);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("Worker%i │ Received %d-byte message from server: \"%s\"\n", function_args->name, recvlen, buf);
      if (strcmp(buf, "Ping")==0) {
        sendto(s, "Pong", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
      char direction[BUFSIZE];
      char args[BUFSIZE];
      sscanf(buf, "%s %s", direction, args);
      if (strcmp(direction, "Map")==0) {
        printf("Worker%i │ Starting map of %s.\n", function_args->name, args);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));

        // TODO Make this size dynamic/configurable. Also change to relative path. Also this maybe shouldn't be done worker-side.

        char content[MAXCONTENT];
        char line[MAXLINE];
        char path[100] = "./program/file_part";
        char* finalpath = strcat(path, args);
        FILE* fp = fopen(finalpath,"r");
        rewind(fp);
        fwrite("\n", sizeof(char), 2, fp);
        while (fgets(line, MAXLINE, fp)) {
          strcat(content, line);
        }
        fclose(fp);

        struct pair file = {finalpath, content};
        results = (function_args->map)(file);

        char wpath[100] = "./program/intermediate";
        char t_name[10];
        sprintf(t_name, "%ld", strtol(args, NULL, 10));
        strcat(wpath, t_name);
        set_output_file(wpath, results, function_args->length);
        printf("Worker%i │ Finished writing to file.\n", function_args->name);
        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
      else if (strcmp(direction, "Reduce") == 0) {
        printf("Worker%i │ Starting reduce.\n", function_args->name);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
        int split_args[3];
        sscanf(args, "%i-%i-%i", &split_args[0], &split_args[1], &split_args[2]);

        // Aggregate intermediate data into one file
        char agg_path[50];
        sprintf(agg_path, "./program/aggregate%d", split_args[0]);
        char* path_base = "./program/intermediate";
        FILE* aggregate = fopen(agg_path, "w");
        aggregate_outputs(aggregate, path_base, split_args[2]);

        // Sort file to group keys
        char sort_path[50];
        sprintf(sort_path, "./program/sorted%d", split_args[1]);
        int sort_len = sort_file(sort_path, agg_path);

        // Run reduce on subsets of keys
        struct pair* in = (struct pair*)malloc(sizeof(struct pair)*(sort_len+1));
        in[sort_len].key = 0x0;
        in[sort_len].value = 0x0;
        retrieve_correct_portion(split_args[1], split_args[0], sort_path, &in, sort_len);
        out = (struct pair*)malloc(sizeof(struct pair)*(sort_len+1));
        out = (function_args->reduce)(in);
        out[sort_len].key = 0x0;
        out[sort_len].value = 0x0;
        // Write output to file
        char out_path[50];
        sprintf(out_path, "./program/out%d", split_args[1]);
        int reduce_len = 0;
        for (int i = 0; out[i].key != 0x0; i++) {
          reduce_len++;
        }
        set_output_file(out_path, out, reduce_len);

        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
      else if (strcmp(direction, "Clean")==0) {
        int split_args[2];
        sscanf(args, "%i-%i", &split_args[0], &split_args[1]);
        FILE* finalagg = fopen("./program/finalaggregate", "w");
        char agg_base[50] = "./program/out";
        aggregate_outputs(finalagg, agg_base, split_args[1]);
        fclose(finalagg);
        (function_args->translate)("./program/finalaggregate");
        /* char final[20] = "./final"; */
        /* sort_file(final, "./finalaggregate"); */
        // cleanup(split_args[0], split_args[1]);
        /* printf("MAINLIB │ \x1B[0;32mTranslation complete.\x1B[0;37m \n"); */
        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
    }
  }
}
