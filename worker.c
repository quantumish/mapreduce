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

void* startWorker(void * name, int (*map)(char*, char*), int (*reduce)(int*))
{
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
  printf("Worker%i | Informed server of existence.\n", name);
  char buf[BUFSIZE];
  int recvlen;
  socklen_t len = sizeof(addr);
  while (1==1)
  {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &len);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("Worker%i | Received %d-byte message from server: \"%s\"\n", name, recvlen, buf);
      char direction[5];
      char args[BUFSIZE];
      sliceString(buf, direction, 0, 5);
      sliceString(buf, args, 6, recvlen-1);
      if (strcmp(direction, "Map---")==0)
      {
        printf("Worker%i | Starting map of %s.\n", name, args);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
        // TODO Make this size dynamic/configurable
        char content[4096]; char line[1024];
        char* path = "/Users/davidfreifeld/file_part"; strcat(path, args);
        FILE *fp = fopen(path,"r");
        while (fgets(line, sizeof line, fp) != NULL)
        {
          strcat(content, line);
        }
        fclose(fp);
      }
    }
  }
  return 1;
}
