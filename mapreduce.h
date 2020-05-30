/** mapreduce.h
 *
 * Main process that is called and manages higher level things
 *
 */

#ifndef MODULE_H
#define MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>

#include "server.h"
#include "worker.h"

#define MAXLINE 1024

void split(char* path, int num_splits);
void begin(char* path, int (*map)(char*), int (*reduce)(int*), int m);


#endif /* main.h */
