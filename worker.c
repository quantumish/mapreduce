#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include "worker.h"

// Used from https://stackoverflow.com/questions/26620388/c-substrings-c-string-slicing
void sliceString(char * str, char * buffer, size_t start, size_t end)
{
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

// This tutorial helped quite a bit in debugging what was going wrong with connection
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

void* startWorker(void* arguments)
{
  struct args *function_args = arguments;
  /* (*function_args->map)("a", "abc"); */
  char* start = "Online.";

  // Same socket is needed on client end so initialize all over again.
  int s;
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    exit(1);
  }
  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; // Specify address family.
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY just 0.0.0.0, machine IP address
  addr.sin_port = htons(PORT); // Specify port.

  // Connect to server
  if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    printf("\n Error : Connect Failed \n");
    exit(0);
  }
  // NOTE This can and will not work if flag argument set to 1
  sendto(s, start, BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
  printf("Worker%i | Informed server of existence.\n", function_args->name);
  char buf[BUFSIZE];
  int recvlen;
  socklen_t len = sizeof(addr);
  while (1==1)
  {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &len);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("Worker%i | Received %d-byte message from server: \"%s\"\n", function_args->name, recvlen, buf);
      char direction[5];
      char args[BUFSIZE];
      sliceString(buf, direction, 0, 5);
      sliceString(buf, args, 6, recvlen-1);
      if (strcmp(direction, "Map---")==0)
      {
        printf("Worker%i | Starting map of %s.\n", function_args->name, args);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));

        // This was miserable. Strings are bothersome enough in C but files take it to a whole new level. Vague segfaults, bus errors, and more.
        // TODO Make this size dynamic/configurable. Also change to relative path. Also this probably shouldn't be done worker-side.
        char content[4096];
        char line[1024];
        char path[100] = "/Users/davidfreifeld/projects/mapreduce/file_part";
        char* finalpath = strcat(path, args);
        printf("Path %s\n", finalpath);
        FILE *fp = fopen(finalpath,"r");
        while (fgets(line, sizeof line, fp) != NULL)
        {
          strcat(content, line);
        }
        fclose(fp);
        int count = 1;
        /* printf("%x \n", sizeo*function_args->map)); */
        /* (*function_args->map)("a", "abc"); */
        printf("Final for a %i \n", count);
        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
    }
  }
  return 1;
}
