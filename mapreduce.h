/** mapreduce.h
 *
 * Main process that is called and manages higher level things
 *
 */

#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include "server.h"
#include "worker.h"

#define MAXLINE 1024

void split(char *path, int num_splits);
void cleanup(int m, int r);

// Starts MapReduce. Calls split function, starts server and worker processes,
// then recieves output of server and returns it back to program.
void begin(char* path, struct pair* (*map)(struct pair), struct pair* (*reduce)(struct pair *), int m, int length, char* ip, int r);

#endif /* main.h */
