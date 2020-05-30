#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>

#include "server.h"
#include "worker.h"
#include "mapreduce.h"

#define MAXLINE 1024

/* Splits input file into num_splits subfiles for parallelization accross computers and CPUs */
void split(char* path, int num_splits)
{
  FILE *rptr;
  rptr = fopen(path, "r");
  if (rptr==NULL) {
    printf("Error! File could not be opened.");
    exit(1);
  }
  char fileoutputname[15];
  char line[MAXLINE];
  FILE *wptr;
  int filecounter=1;
  int linecounter=1;
  int linecount=1;
  sprintf(fileoutputname, "file_part%d", filecounter);
  wptr = fopen(fileoutputname, "w");
  while (fgets(line, sizeof line, rptr) != NULL) {
    linecount++;
  }
  rewind(rptr);
  // Based off of https://www.codingunit.com/c-tutorial-splitting-a-text-file-into-multiple-files
  while (fgets(line, sizeof line, rptr) != NULL) {
    if (linecounter == round((float) linecount / (float) num_splits)) {
      fclose(wptr);
      linecounter = 1;
      filecounter++;
      sprintf(fileoutputname, "file_part%d", filecounter);
      wptr = fopen(fileoutputname, "w");
      if (!wptr)
        exit(1);
    }
    fprintf(wptr,"%s\n", line);
    linecounter++;
  }
  fclose(rptr);
}

void begin(char* path, int (*map)(char*, char*), int (*reduce)(int*), int m)
{
  split(path, m);
  // Run the server as well as 7 workers. Server is the only thread that returns, so
  // no joining is necessary. Source for POSIX threading library:
  // https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html
  pthread_t thread1, thread2, thread3, thread4, thread5, thread6, thread7, thread8;
  int ret1;
  ret1 = pthread_create(&thread1, NULL, startServer, (void *) m);
  pthread_create(&thread2, NULL, startWorker, NULL);
}
