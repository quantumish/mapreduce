#include "server.h"

#define BUFSIZE 2048
#define PORT 5000


// Used from https://stackoverflow.com/questions/26620388/c-substrings-c-string-slicing
void sliceString(char * str, char * buffer, size_t start, size_t end)
{
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

// Starts a UDP server which then listens for workers coming online, updates lookup table on
// worker availability, and instructs workers on what to do. General reference for the UDP server:
// https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
void* startServer(void* m)
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
  int deviceCounter = 0;

  while (1==1)
  {
    // TODO Remove this.
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf(" SERVER | Received %d-byte message from %i: \"%s\"\n", recvlen, remaddr.sin_port, buf);
      // TODO Switch to hashmap instead of my bad version of Python dictionaries
      int keys[500];
      char* values[500];
      char* response = "Acknowledged.";
      if (strcmp(buf, "Online.")==0)
      {
        /* response = (char*) remaddr.sin_port; */
        sendto(s, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);
        keys[deviceCounter] = remaddr.sin_port;
        values[deviceCounter] = "Idle";
        deviceCounter+=1;
      }
      if (strcmp(buf, "Starting.")==0)
      {
        sendto(s, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);
        for (int i = 0; i <= deviceCounter; i++)
        {
          if (keys[i] == remaddr.sin_port)
          {
            values[i] = "In-Progress";
          }
        }
      }
      if (strcmp(buf, "Done")==0)
      {
        sendto(s, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);
        for (int i = 0; i <= deviceCounter; i++)
        {
          if (keys[i] == remaddr.sin_port)
          {
            values[i] = "Idle";
          }
        }
      }
      for (int i = 0; values[i] != NULL; i++)
      {
        if (strcmp(values[i], "Idle")==0)
        {
          char order[6] = "map";
          /* Specify who we're talking to, as its more than just whoever talked to server */
          /* int s2; */
          /* if ((s2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) */
          /* { */
          /*   exit(1); */
          /* } */
          /* struct sockaddr_in outaddr; */
          /* memset((char *)&outaddr, 0, sizeof(outaddr)); */
          /* outaddr.sin_family = AF_INET; */
          /* outaddr.sin_addr.s_addr = htonl(INADDR_ANY); */
          /* outaddr.sin_port = htons(keys[i]); */
          /* if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) */
          /* { */
          /*   printf("\n Error : Connect Failed \n"); */
          /*   exit(0); */
          /* } */
          sendto(s, order, strlen(order), 0, (struct sockaddr *) &remaddr, addrlen);
        }
      }
    }
  }
}
