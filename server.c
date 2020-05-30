#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>

#define BUFSIZE 2048
#define PORT 5000

// General reference for the UDP server:
// https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
int main()
{
  // Socket being created
  int s;
  // Intialize socket with AF_INET IP family and SOCK_DGRAM datagram service, exit if failed
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    exit(1);
  }
  // Establish sockaddr_in struct to pass into bind function
  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET; // Specify address family.
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY just 0.0.0.0, machine IP address
  addr.sin_port = htons(PORT); // Specify port.
  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    exit(1);
  }
  // IP address of client
  struct sockaddr_in remaddr;
  // Size of address
  socklen_t addrlen = sizeof(remaddr);
  // Specify max bytes recieved/transferred
  int recvlen;
  // Buffer to store recieved data
  unsigned char buf[BUFSIZE];
  while (1==1)
  {
    printf("Waiting on port %d\n", PORT);
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    printf("received %d bytes\n", recvlen);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("received message: \"%s\"\n", buf);
    }
    // use sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen) to send things
  }
}
