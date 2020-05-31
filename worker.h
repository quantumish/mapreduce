/** worker.h
 *
 * Worker process responsible for executing map and reduce functions
 *
 */

#ifndef WORKER_H
#define WORKER_H

#define PORT 5000
#define BUFSIZE 2048

struct args
{
  int name;
  int (*map)(char*, char*);
  int (*reduce)(int*);
};

void* startWorker(void* arguments);

#endif /* MODULE_H */
