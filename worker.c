#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include "worker.h"

// FIXME DESTROY ME I SHOULD NOT EXIST CAST ME INTO THE FLAMES I AM A SYMPTOM OF TERRIBLE CODE
// Used from https://stackoverflow.com/questions/26620388/c-substrings-c-string-slicing

static void set_output_file(char* path, struct int_pair * pair_list, int length)
{
  FILE * wptr;
  wptr = fopen(path, "w");
  for (int i = 0; i < length; i++) {
    fprintf(wptr, "%s %i\n", pair_list[i].key, pair_list[i].value);
  }
  fclose(wptr);
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
        sprintf(s_name, "%d", strtol(args, NULL, 10));
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

        for (int i = 0; i < split_args[0]; i++) {
          char* path_base = "/Users/davidfreifeld/projects/mapreduce/intermediate";
          char s_name[10];
          sprintf(s_name, "%d", i);
          char path [strlen(path_base) + strlen(s_name) + 1];
          strcpy(path, path_base);
          strcat(path, s_name);
        }

        /* int totalsize = 0; */
        /* char* paths[2]; */
        /* for (int i = 0; i < 2; i++) { */
        /*   char temp_path[100]; */
        /*   strcpy(temp_path, path_base); */
        /*   strcat(temp_path, split_args[i]); */
        /*   paths[i] = temp_path; */

        /*   FILE* fptr = fopen(paths[i], "r"); */
        /*   fseek(fptr, 0L, SEEK_END); */
        /*   int sz = ftell(fptr); */
        /*   totalsize+=sz; */
        /* } */
        /* printf("path: %s\n", paths[0]); */
        struct int_pair* input = malloc(sizeof(struct int_pair)*(26)); // -2 is to ignore null-terminating chars

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
