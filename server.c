#include "server.h"

#define BUFSIZE 2048
#define PORT 5000

// Starts a UDP server which then listens for workers coming online, updates lookup table on
// worker availability, and instructs workers on what to do. General reference for the UDP server:
// https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
void start_server(void* server_arguments)
{
  struct server_args* function_args = (struct server_args*) server_arguments;
  printf(" SERVER │ \x1B[0;32mServer online.\x1B[0;37m\n");
  // Intialize socket with AF_INET IP family and SOCK_DGRAM datagram service, exit if failed
  int s;
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
  socklen_t addrlen = sizeof(remaddr);
  int recvlen;
  unsigned char buf[BUFSIZE];

  // List to track which files have been mapped. 0 = unmapped, 1 = mapped, -1 = in-progress.
  int* mapped = malloc(function_args->m * sizeof(int));
  int* reduced = malloc(function_args->r * sizeof(int));
  for (int i = 0; i < function_args->r; i++) {
    reduced[i] = 0;
  }

  // Initialize mapped list to prevent bugs
  for (int i = 0; i < function_args->m; i++)
  {
    mapped[i] = 0;
  }

  int clean_status = 0;

  // TODO Remove this
  int deviceCounter = 0;
  int phase = 0; // Prevent attempting to order mapping during reduce stage.

  // Define poor data structure for remembering states of clients. Define struct to allow storing of info about clients.
  // TODO Switch to hashmap instead of my bad version of Python dictionaries
  int keys[500];
  struct client values[500];
  while (1==1) {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf(" SERVER │ Received %d-byte message from %i: \"%s\"\n", recvlen, remaddr.sin_port, buf);
      // TODO Condense this somehow. Helper functions feels a little bizarree for this and each is different so not sure how to.
      if (strcmp(buf, "Online.")==0) {
        keys[deviceCounter] = remaddr.sin_port;
        values[deviceCounter].status = "Idle";
        deviceCounter+=1;
      }
      if (strcmp(buf, "Starting.")==0) {
        for (int i = 0; i <= deviceCounter; i++) {
          if (keys[i] == remaddr.sin_port) {
            values[i].status = "In-Progress";
          }
        }
      }
      if (strcmp(buf, "Done.")==0) {
        for (int i = 0; i <= deviceCounter; i++) {
          if (keys[i] == remaddr.sin_port) {
            values[i].status = "Idle";
            if (phase == 0) mapped[values[i].assigned] = 1;
            if (phase == 1) reduced[values[i].assigned] = 1;
            if (phase == 2) clean_status = 1;
            values[i].assigned = -1;
            break;
          }
        }
      }
      if (phase == 0)
      {

        // HACK Find a better way for checking in-progress

        int target = -1;
        int prog = -1;
        for (int j = 0; j < function_args->m; j++) {
          if (mapped[j] == 0) {
            target = j;
            break;
          }
          if (mapped[j] == 0 || mapped [j] == -1) {
            prog = 1;
          }
        }
        if (target != -1) {
          for (int i = 0; values[i].status != NULL; i++) {
            if (strcmp(values[i].status, "Idle")==0) {
              char* order = (char*)malloc(13*sizeof(char));
              remaddr.sin_port = keys[i];
              sprintf(order, "Map %i", target);
              sendto(s, order, strlen(order), 0, (struct sockaddr *) &remaddr, addrlen);
              values[i].status = "Waiting";
              values[i].assigned = target;
              mapped[values[i].assigned] = -1;
              break;
            }
          }
        }
        else {
          if (prog == -1) {
            printf(" SERVER │ \x1B[0;32mMapping complete.\x1B[0;37m \n");
            phase = 1;
            // HACK Prevent stall
            sendto(s, "Ping", 4, 0, (struct sockaddr *) &remaddr, addrlen);
          }
        }
      }
      else if (phase == 1) {
        int target = -1;
        int prog = -1;
        /* printf("Values are..."); */
        for (int i = 0; i < function_args->r; i++) {
          /* printf("%i\n", reduced[i]); */
          if (reduced[i] == 0) {
            target = i;
            break;
          }
          if (reduced[i] == 0 || reduced[i] == -1) {
            prog = 1;
          }
        }
        /* printf("\n\n\n"); */
        if (target != -1) {
          for (int i = 0; values[i].status != NULL; i++) {
            if (strcmp(values[i].status, "Idle") == 0) {
              char *order = (char *)malloc(13 * sizeof(char));
              sprintf(order, "Reduce %i-%i-%i", function_args->r, target, function_args->m);
              remaddr.sin_port = keys[i];
              sendto(s, order, strlen(order)+1, 0, (struct sockaddr *)&remaddr, addrlen);
              values[i].status = "Waiting";
              values[i].assigned = target;
              reduced[target] = -1;
              break;
            }
          }
        }
        else if (prog == -1) {
          printf(" SERVER │ \x1B[0;32mReducing complete.\x1B[0;37m \n");
          phase = 2;
        }
      }
      if (phase == 2 && clean_status != 1) {
        if (clean_status != -1) {
          char *order = (char *)malloc(13 * sizeof(char));
          sprintf(order, "Clean %i-%i", function_args->m, function_args->r);
          remaddr.sin_port = keys[0];
          sendto(s, order, strlen(order)+1, 0, (struct sockaddr *)&remaddr, addrlen);
          values[0].status = "Waiting";
          clean_status = -1;
        }
      }
    }
  }
  free(mapped);
  free(reduced);
}
