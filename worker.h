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

struct args
{
  int name;
  int (*map)(char*);
  int (*reduce)(int*);
};

void sliceString(char * str, char * buffer, size_t start, size_t end);
void* startWorker(void* arguments);

#endif /* MODULE_H */
