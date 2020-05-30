#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <arpa/inet.h>


#define BUFSIZE 2048
#define PORT 5000


// Adapted from https://stackoverflow.com/questions/26620388/c-substrings-c-string-slicing
char* getUUID(char * str, char * buffer, size_t start, size_t end)
{
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

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
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      if (strcmp(buf, "Online."))
      {
        char id[2048];
        getUUID(buf, id, 0, 37);
        printf("Received %d-byte message from %s: \"%s\"\n", recvlen, id, buf);
        strcpy(buf, "Acknowledged.");
        sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen);
      }
    }
  }
}