#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 5000
#define BUFSIZE 2048


// This tutorial helped quite a bit in debugging what was going wrong with connection
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
int main ()
{
  char* alive = "Alive.";

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
  sendto(s, alive, BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
  printf("Informed server of existence.\n");
  char buf[BUFSIZE];
  int recvlen;
  socklen_t len = sizeof(addr);
  while (1==1)
  {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &len);
    printf("received %d bytes\n", recvlen);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("received message: \"%s\"\n", buf);
    }
  }
}
