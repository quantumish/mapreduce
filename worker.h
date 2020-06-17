/** worker.h
 *
 * Worker process responsible for executing map and reduce functions
 *
 */

#ifndef WORKER_H
#define WORKER_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h>

#define PORT 5000
#define BUFSIZE 2048
#define MAXLINE 1024
#define MAXCONTENT 1000000 // HACK Make this dynamic in some way.

// Key-value-pair-esque structures.
struct pair
{
  void* key;
  void* value;
};

// Structure for circumventing threading function issues.
struct args
{
  int name;
  int length;
  int ip;
  struct pair* (*map)(struct pair);
  struct pair* (*reduce)(struct pair*);
};

// All non-static functions.
void aggregate_outputs(FILE* final, char* path_base, int max_name);
int sort_file(char* finalpath, char* path);
void* start_worker(void* arguments);

#endif /* MODULE_H */
