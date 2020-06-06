/** master.h
 *
 * Master process responsible for directing other processes.
 *
 */

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <arpa/inet.h>

// pthreads aggravates me
struct server_args
{
  int devices;
  int reducers;
  int m;
};

// Bundle client info.
struct client
{
  char* status;
  int assigned;
};

void start_server(void* arguments);

#endif /* MODULE_H */
