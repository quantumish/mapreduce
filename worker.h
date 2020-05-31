/** worker.h
 *
 * Worker process responsible for executing map and reduce functions
 *
 */

#ifndef WORKER_H
#define WORKER_H

#define PORT 5000
#define BUFSIZE 2048

void* startWorker(void * name, int (*map)(char*, char*), int (*reduce)(int*));

#endif /* MODULE_H */
