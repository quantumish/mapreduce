#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h>

#include "worker.h"

static void set_output_file(char* path, struct int_pair * pair_list, int length)
{
  FILE * wptr;
  wptr = fopen(path, "w");
  for (int i = 0; i < length; i++) {
    fprintf(wptr, "%s %i\n", pair_list[i].key, pair_list[i].value);
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

int sort_file(char* finalpath, char* path, int name)
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
    list[i] = malloc(sizeof(char)*MAXLINE);
    fgets(list[i], BUFSIZE, input);
  }
  qsort(list, length, sizeof (char*), alphcmp);
  char s_name[10];
  sprintf(s_name, "%d", name);
  strcat(finalpath, s_name);
  FILE* sorted = fopen(finalpath, "w");
  for (int i = 0; i < length; i++) {
    fprintf(sorted, "%s", list[i]);
  }
  fclose(sorted);
  fclose(input);
  return length;
}


static void get_output_file_portion(FILE* fp, struct int_pair* pair_list, int m, int n)
{
  char line[MAXLINE];
  int i = 0;
  int j = 0;
  rewind(fp);
  while (fgets(line, MAXLINE, fp) != NULL) {
    if ((i >= m) && (i < n)) {
      struct int_pair *ret = malloc(sizeof(struct int_pair));
      ret->key = malloc(sizeof(char) * MAXLINE);
      sscanf(line, "%s%d", ret->key, &(ret->value));
      pair_list[j] = *ret;
      j++;
    }
    i++;
  }
}

static void retrieve_correct_portion(long piece, long total, char* sorted_path, struct int_pair** input, long length) {
  struct int_pair * pair_list = *input;
  /* for (int i = 0; i < 1; i++) { */
  /*   printf("%s %i\n", pair_list[i].key, pair_list[i].value); */
  /* } */
  FILE* fptr = fopen(sorted_path, "r");
  char* prevline = malloc(MAXLINE * sizeof(char));
  char* line = malloc(MAXLINE * sizeof(char));
  int i = 0;
  int range[2] = {length, 0};
  while (fgets(line, sizeof(line), fptr)) {
    char* key  = malloc(MAXLINE * sizeof(char));
    sscanf(line, "%s *s", key);
    strcpy(line, key);
    if ((i >= (piece*length)/total) && (i <= ((1+piece)*length)/total) && (strcmp(line, prevline)!=0)) {
      if (i < range[0]) range[0] = i;
      else if (i > range[1]) range[1] = i;
    }
    strcpy(prevline, line);
    i++;
  }
  if ((i >= (piece*length)/total) && (i <= ((1+piece)*length)/total)) {
    if (i > range[1]) range[1] = i;
  }
  get_output_file_portion(fptr, pair_list, range[0], range[1]);
  pair_list[range[1]-range[0]].key = '\0';
  pair_list[range[1]-range[0]].value = -1;
  /* for (int i = 0; i < (range[1])-range[0] + 1; i++) { */
  /*   printf("%i %s %i\n", i, pair_list[i].key, pair_list[i].value); */
  /* } */
}
// This tutorial helped quite a bit in debugging what was going wrong with connection
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

void* startWorker(void* arguments)
{
  struct args *function_args = (struct args *)arguments;
  char* start = "Online.";
  char s_name[10];
  sprintf(s_name, "%d", function_args->name);

  // Same socket is needed on client end so initialize all over again.
  int s;
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("\n Error : Socket Failed \n");
  }
  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; // Specify address family.
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY just 0.0.0.0, machine IP address
  addr.sin_port = htons(PORT); // Specify port.

  // Connect to server
  if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("\n Error : Connect Failed \n");
  }
  // NOTE This can and will not work if flag argument set to 1
  sendto(s, start, BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
  printf("Worker%i | Informed server of existence.\n", function_args->name);
  char buf[BUFSIZE];
  int recvlen;
  socklen_t len = sizeof(addr);

  while (1==1) {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &len);

    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("Worker%i | Received %d-byte message from server: \"%s\"\n", function_args->name, recvlen, buf);
      if (strcmp(buf, "Ping")==0) {
        sendto(s, "Pong", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
      char direction[7]; // Direction is either Map or Reduce so longer than 7 (to compensate for \0) is not needed
      char args[BUFSIZE];
      sscanf(buf, "%s %s", direction, args);
      if (strcmp(direction, "Map")==0) {
        printf("Worker%i | Starting map of %s.\n", function_args->name, args);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));

        // TODO Make this size dynamic/configurable. Also change to relative path. Also this maybe shouldn't be done worker-side.

        char content[4096];
        char line[1024];
        char path[100] = "./file_part";
        char* finalpath = strcat(path, args);
        FILE* fp = fopen(finalpath,"r");
        while (fgets(line, sizeof line, fp) != NULL) {
          printf("%s:%s",finalpath,line);
          strcat(content, line);
        }
        fclose(fp);

        struct str_pair file = {finalpath, content};
        struct int_pair * results = (struct int_pair *)malloc(sizeof(struct int_pair)*function_args->length);
        results = (*function_args->map)(file);

        char wpath[100] = "./intermediate";
        char t_name[10];
        sprintf(t_name, "%ld", strtol(args, NULL, 10));
        strcat(wpath, t_name);
        set_output_file(wpath, results, function_args->length);
        printf("Worker%i | Finished writing to file.\n", function_args->name);
        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
      else if (strcmp(direction, "Reduce") == 0) {
        printf("Worker%i | Starting reduce.\n", function_args->name);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
        int split_args[2];
        sscanf(args, "%i-%i", &split_args[0], &split_args[1]);

        char agg_path[20] = "./aggregate";
        strcat(agg_path, s_name);
        char* path_base = "./intermediate";
        FILE* aggregate = fopen(agg_path, "w");
        aggregate_outputs(aggregate, path_base, split_args[0]);

        char sort_base[20] = "./sorted";
        strcat(sort_base, s_name);
        int sort_len = sort_file(sort_base, agg_path, function_args->name);

        struct int_pair* in = malloc(sizeof(struct int_pair)*(sort_len+1));
        retrieve_correct_portion(split_args[1], split_args[0], sort_base, &in, sort_len);
        struct int_pair* out = malloc(sizeof(struct int_pair)*(sort_len+1));
        out = (*function_args->reduce)(in);
        
        char out_path[20] = "./out";
        char str_assigned[10];
        sprintf(str_assigned, "%d", split_args[1]);
        strcat(out_path, str_assigned);
        int reduce_len = 0;
        for (int i = 0; out[i].key != NULL; i++) {
          reduce_len++;
        }
        
        set_output_file(out_path, out, reduce_len);
        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
    }
  }
}
