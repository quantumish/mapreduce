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

char* sliceString(char * str, char * buffer, size_t start, size_t end);
void* startServer(void* m);

#endif /* MODULE_H */
