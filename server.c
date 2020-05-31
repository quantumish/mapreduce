#include "server.h"

#define BUFSIZE 2048
#define PORT 5000

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
  // List to track which files have been mapped. 0 = unmapped, 1 = mapped, -1 = in-progress.
  int mapped[(int) m];
  // Initialize mapped list to prevent bugs
  for (int i = 0; i < (int) m; i++)
  {
    mapped[i] = 0;
  }
  // TODO Remove this
  int deviceCounter = 0;
  // Define poor data structure for remembering states of clients. Define struct to allow storing of info about clients.
  struct client
  {
    char* status;
    int assigned;
  };
  int keys[500];
  struct client values[500];
  while (1==1)
  {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf(" SERVER | Received %d-byte message from %i: \"%s\"\n", recvlen, remaddr.sin_port, buf);
      // TODO Switch to hashmap instead of my bad version of Python dictionaries
      char* response = "Acknowledged.";
      if (strcmp(buf, "Online.")==0)
      {
        /* sendto(s, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen); */
        keys[deviceCounter] = remaddr.sin_port;
        values[deviceCounter].status = "Idle";
        deviceCounter+=1;
      }
      if (strcmp(buf, "Starting.")==0)
      {
        /* sendto(s, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen); */
        for (int i = 0; i <= deviceCounter; i++)
        {
          if (keys[i] == remaddr.sin_port)
          {
            values[i].status = "In-Progress";
          }
        }
      }
      if (strcmp(buf, "Done")==0)
      {
        /* sendto(s, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen); */
        for (int i = 0; i <= deviceCounter; i++)
        {
          if (keys[i] == remaddr.sin_port)
          {
            values[i].status = "Idle";
            mapped[values[i].assigned] = 1;
            values[i].assigned = -1;
            break;
          }
        }
      }
      int target = -1;
      for (int j = 0; j < (int) m; j++)
      {
        if (mapped[j] == 0)
        {
          target = j;
          break;
        }
      }
      if (target != -1)
      {
        for (int i = 0; values[i].status != NULL; i++)
        {
          if (strcmp(values[i].status, "Idle")==0)
          {
            char* order = (char*)malloc(13*sizeof(char));
            sprintf(order, "Map---%i", target);
            sendto(s, order, strlen(order), 0, (struct sockaddr *) &remaddr, addrlen);
            values[i].status = "Waiting";
            values[i].assigned = target;
            mapped[values[i].assigned] = -1;
          }
        }
      }
      else
      {
        // Reduce.
      }
    }
  }
}
