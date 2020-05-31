/** worker.h
 *
 * Worker process responsible for executing map and reduce functions
 *
 */

#ifndef WORKER_H
#define WORKER_H

#include <sys/types.h>

#define PORT 5000
#define BUFSIZE 2048


struct str_pair
{
  char* key;
  char* value;
};

struct int_pair
{
  char* key;
  int value;
};

struct args
{
  int name;
  struct int_pair* (*map)(struct str_pair);
  struct int_pair (*reduce)(struct int_pair*);
};


void sliceString(char * str, char * buffer, size_t start, size_t end);
void* startWorker(void* arguments);

#endif /* MODULE_H */
