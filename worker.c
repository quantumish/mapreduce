#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <uuid/uuid.h>

#include "worker.h"

// This tutorial helped quite a bit in debugging what was going wrong with connection
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

void* startWorker(void * unused)
{
  uuid_t binuuid;
  uuid_generate_random(binuuid);
  char* uuid = malloc(BUFSIZE);
  uuid_unparse_lower(binuuid, uuid);
  printf("UUID: %s\n", uuid);
  
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
  strcat(uuid, ": ");
  char startmsg[BUFSIZE];
  strcpy(startmsg, uuid);
  strcat(startmsg, start);

  sendto(s, startmsg, BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
  printf("Informed server of existence.\n");
  char buf[BUFSIZE];
  int recvlen;
  socklen_t len = sizeof(addr);
  while (1==1)
  {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &len);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("Received %d-byte message from master: \"%s\"\n", recvlen, buf);
      if (strcmp(buf, "Acknowledged.")==0)
      {

      }
    }
  }
  return 1;
}
