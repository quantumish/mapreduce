#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <execinfo.h>
#include <unistd.h>

#include "server.h"
#include "worker.h"
#include "mapreduce.h"

#define MAXLINE 1024

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 12);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

/* Splits input file into num_splits subfiles for parallelization accross computers and CPUs */
void split(char* path, int num_splits) {
  printf("MAINLIB | Beginning split of file into %i pieces.\n", num_splits);

  FILE *rptr;
  rptr = fopen(path, "r");
  if (rptr==NULL) {
    printf("Error! File could not be opened.");
    exit(1);
  }
  char fileoutputname[15];
  char line[MAXLINE];
  FILE *wptr;
  int filecounter=0;
  int linecounter=0;
  int linecount=0;
  sprintf(fileoutputname, "file_part%d", filecounter);
  wptr = fopen(fileoutputname, "w");
  while (fgets(line, sizeof line, rptr) != NULL) {
    linecount++;
  }
  rewind(rptr);
  // Based off of https://www.codingunit.com/c-tutorial-splitting-a-text-file-into-multiple-files
  printf("Split line at %d vs\n",  linecount /  num_splits);
  while (fgets(line, sizeof line, rptr) != NULL) {
    if (linecounter == linecount /  num_splits) {
      fclose(wptr);
      linecounter = 0;
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
  printf("MAINLIB | \x1B[0;32mSplitting complete.\x1B[0;37m \n");
}

void begin(char* path, struct int_pair * (*map)(struct str_pair), struct int_pair * (*reduce)(struct int_pair *), int m, int length)
{
  // Intialize error handler to spare my sanity when the inevitable segfault occurs
  signal(SIGSEGV, handler);
  // Split file into M pieces as detailed in paper.
  split(path, m);
  // Run the server as well as 7 workers. Server is the only thread that returns, so
  // no joining besides that thread is necessary. Source for POSIX threading library:
  // https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html

  pthread_t server;
  int ret1;
  ret1 = pthread_create(&server, NULL, startServer, (void *) m);
  printf("MAINLIB | Created server thread.\n");


  for (int i = 0; i < m; i++) {
    pthread_t worker;
    // Trickery with structs as pthread_create only allows one argument to function for some reason.
    struct args * pass_args = malloc(sizeof(struct args));
    pass_args->name = i;
    pass_args->map = map;
    pass_args->reduce = reduce;
    pass_args->length = length;
    pthread_create(&worker, NULL, startWorker, (void *) pass_args);
    printf("MAINLIB | Created worker thread #%i.\n", i);

  }
  pthread_join(server, NULL);

  FILE* finalagg = fopen("./finalaggregate", "w");
  char agg_base[20] = "./out";
  aggregate_outputs(finalagg, agg_base, (int)m);
  char final[20] = "./final";
  sort_file(final, "./finalaggregate", 0);
  printf("MAINLIB | \x1B[0;32mComplete!\x1B[0;37m \n");
}
