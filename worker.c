#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include "worker.h"

// TODO Remove me
// Used from https://stackoverflow.com/questions/26620388/c-substrings-c-string-slicing
void slice_string(char * str, char * buffer, size_t start, size_t end)
{
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

char* substr(const char *src, int m, int n)
{
    int len = n - m;
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
    for (int i = m; i < n && (*(src + i) != '\0'); i++) {
        *dest = *(src + i);
        dest++;
    }
    *dest = '\0';
    return dest - len;
}

// This tutorial helped quite a bit in debugging what was going wrong with connection
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

void* startWorker(void* arguments)
{
  struct args *function_args = (struct args *)arguments;
  /* function_args->name *= rand(); */
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
      char direction[7];
      char args[BUFSIZE];
      strcpy(direction, substr(buf, 0, 6));
      slice_string(buf, args, 6, recvlen-1);
      if (strcmp(direction, "Map---")==0) {
        printf("Worker%i | Starting map of %s.\n", function_args->name, args);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));

        // This was miserable. Strings are bothersome enough in C but files take it to a whole new level. Vague segfaults, bus errors, and more.
        // TODO Make this size dynamic/configurable. Also change to relative path. Also this maybe shouldn't be done worker-side.

        char content[4096]; 
        char line[1024];
        char path[100] = "/Users/davidfreifeld/projects/mapreduce/file_part";
        char* finalpath = strcat(path, args);
        FILE* fp = fopen(finalpath,"r");
        while (fgets(line, sizeof line, fp) != NULL) {
          strcat(content, line);
        }
        fclose(fp);
        struct int_pair results[function_args->length];
        struct str_pair file = {finalpath, content};
        struct int_pair map_output = *(*function_args->map)(file);
        memcpy(&results, &map_output, sizeof(map_output));
        printf("Worker%i | Second intermediate value %i vs %i for part %s\n", function_args->name, results[1].value, args);
        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));

        FILE* wptr;
        char wpath[100] = "/Users/davidfreifeld/projects/mapreduce/intermediate";
        char s_name[10];
        sprintf(s_name, "%d", function_args->name);
        strcat(wpath, s_name);
        wptr = fopen(wpath, "w");
        for (int i = 0; i < function_args->length; i++) {
          printf("Writing '%i'\n", results[i].value);
          /* fprintf(wptr, "%s %i\n", results[i].key, results[i].value); */
        }
        exit(1);
        fclose(wptr);
      }
    }
  }
}
