#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

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

static void aggregate_outputs(FILE* final, int max_name)
{
  char line[BUFSIZE];
  for (int i = 0; i < max_name; i++) {
    char* path_base = "/Users/davidfreifeld/projects/mapreduce/intermediate";
    char s_name[10];
    sprintf(s_name, "%d", i);
    char path [strlen(path_base) + strlen(s_name) + 1];
    strcpy(path, path_base);
    strcat(path, s_name);
    FILE* fptr = fopen(path, "r");
    // Modified from https://stackoverflow.com/questions/11384032/merge-n-files-using-a-c-program/11384194
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

static void sort_file(FILE* final, FILE* input)
{
  int lines = 0;
  char line[MAXLINE];
  /* rewind(input); */
  /* while (fgets(line, sizeof(line), input)) { */
  /*   lines++; */
  /* } */
  while (EOF != (fscanf(input, "%*[^\n]"), fscanf(input,"%*c")))
    ++lines;
  printf("? %i\n", lines);
  rewind(input);
  char* list[lines];
  for (int i = 0; i < lines; i++)  {
    fread(line, sizeof(char), BUFSIZE, input);
    list[i] = line;
  }
  qsort(list, lines, sizeof (char*), alphcmp);
  for (int i = 0; i < lines; i++) {
    printf("%s\n", list[i]);
  }
  /* fwrite(line, sizeof(char), write_sz, final); */
}


static void get_output_file_portion(FILE* fp, struct int_pair* pair_list, int m, int n)
{
  char line[MAXLINE];
  int i = 0;
  while (fgets(line, MAXLINE, fp) != NULL) {
    if ((i >= m) && (i < n)) {
      struct int_pair *ret = malloc(sizeof(struct int_pair));
      ret->key = malloc(sizeof(char) * MAXLINE);
      sscanf(line, "%s%d", ret->key, &(ret->value));
      printf("IN FUNC %s %d\n", ret->key, ret->value);
      pair_list[i] = *ret;
    }
    i++;
  }
}

// This tutorial helped quite a bit in debugging what was going wrong with connection
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

void* startWorker(void* arguments)
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

        // This was miserable. Strings are bothersome enough in C but files take it to a whole new level. Vague segfaults, bus errors, and more.
        // TODO Make this size dynamic/configurable. Also change to relative path. Also this maybe shouldn't be done worker-side.

        char content[4096]; 
        char line[1024];
        char path[100] = "./file_part";
        char* finalpath = strcat(path, args);
        FILE* fp = fopen(finalpath,"r");
        while (fgets(line, sizeof line, fp) != NULL) {
          strcat(content, line);
        }
        fclose(fp);

        struct str_pair file = {finalpath, content};
        struct int_pair * results = (struct int_pair *)malloc(sizeof(struct int_pair)*function_args->length);
        results = (*function_args->map)(file);

        char wpath[100] = "./intermediate";
        char s_name[10];
        sprintf(s_name, "%ld", strtol(args, NULL, 10));
        strcat(wpath, s_name);
        set_output_file(wpath, results, function_args->length);
        printf("Worker%i | Finished writing to file.\n", function_args->name);
        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
      else if (strcmp(direction, "Reduce") == 0) {
        printf("Worker%i | Starting reduce.\n", function_args->name);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));

        int split_args[2];
        sscanf(args, "%i-%i", &split_args[0], &split_args[1]);

        FILE* aggregate = fopen("./aggregate0", "w");
        aggregate_outputs(aggregate, split_args[0]);

        FILE* sorted = fopen("./sorted0", "w");
        sort_file(sorted, aggregate);

        struct int_pair* input = malloc(sizeof(struct int_pair)*(26));
        FILE* fptr = fopen("./intermediate0", "r");
        get_output_file_portion(fptr, input, 0, 5);
        input[1].key = "A";
        for (int i = 0; i < 5; i++) {
          printf("%s %i\n", input[i].key, input[i].value);
        }
        struct int_pair* results; //= malloc(sizeof(struct int_pair)*(26));
        results = (*function_args->reduce)(input);
        for (int i = 0; i < 5; i++) {
          printf("%s %i\n", results[i].key, results[i].value);
        }
      }
    }
  }
}
