/** worker.h
 *
 * Worker process responsible for executing map and reduce functions
 *
 */

#ifndef WORKER_H
#define WORKER_H

#define PORT 5000
#define BUFSIZE 2048

void* startWorker(void * unused);

#endif /* MODULE_H */
