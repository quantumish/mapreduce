#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>

#include "server.h"
#include "worker.h"
#include "mapreduce.h"

#define MAXLINE 1024

// Error handling based off of https://stackoverflow.com/questions/77005/how-to-automatically-generate-a-stacktrace-when-my-program-crashes
// and some man pages. Simply dumps a minimal backtrace to allow me to spare sanity when trying to quickly debug (although lldb is preferable)
void handler(int sig) {
  void *array[10];
  size_t size;
  size = backtrace(array, 12);
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

// Very minimal split function that reads specified file and writes it into separate output files beginning with "file_part"
// Takes char* to path of file to be split and int of number of pieces to split into.
// NOTE/TODO Could be causing bug, tweak me.
void split(char* path, int num_splits) {
  printf("MAINLIB | Beginning split of file into %i pieces.\n", num_splits);
  FILE *rptr;
  rptr = fopen(path, "r");
  if (rptr==NULL) {
    printf("MAINLIB | Error! File could not be opened.");
    exit(1);
  }
  char line[MAXLINE];
  int linecounter, filecounter, linecount;
  linecount=filecounter=linecounter=0;
  char fileoutputname[15];
  sprintf(fileoutputname, "./file_part%d", filecounter);
  FILE *wptr;
  wptr = fopen(fileoutputname, "w");
  while (fgets(line, sizeof line, rptr) != NULL) {
    linecount++;
  }
  printf("LINECOUNT: %i\n", linecount);
  rewind(rptr);
  // Based off of https://www.codingunit.com/c-tutorial-splitting-a-text-file-into-multiple-files
  printf("MAINLIB | Split line at %d vs\n",  linecount / num_splits);
  while (fgets(line, sizeof line, rptr) != NULL) {
    if (linecounter == linecount /  num_splits) {
      fclose(wptr);
      linecounter = 0;
      filecounter++;
      sprintf(fileoutputname, "./file_part%d", filecounter);
      wptr = fopen(fileoutputname, "w");
      if (!wptr)
        exit(1);
    }
    fprintf(wptr,"%s\n", line);
    linecounter++;
  }
  fclose(wptr);
  fclose(rptr);
  printf("MAINLIB | \x1B[0;32mSplitting complete.\x1B[0;37m \n");
}

// Gateway function between user code and MapReduce. Starts up threads for workers and server.
// Takes char* path to input file, function pointer for map function, function pointer for reduce function, int M, and int length of keys
// as well as char* public ip
// TODO Investigate necessity of length parameter and try to get rid of it.
void begin(char* path, struct int_pair * (*map)(struct str_pair), struct int_pair * (*reduce)(struct int_pair *), int m, int length, int devices)
{
  signal(SIGSEGV, handler);
  split(path, m);

  // Run the server as well as 7 workers. Server is the only thread that returns, so
  // no joining besides that thread is necessary. Source for POSIX threading library:
  // https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html

  FILE* fp;
  path[1035];
  fp = popen("/usr/bin/dig +short myip.opendns.com @resolver1.opendns.com", "r");
  if (fp == NULL) {
    printf("Failed to determine IP with dig\n" );
    exit(1);
  }
  char comp_ip[15];
  while (fgets(path, sizeof(path), fp) != NULL) {
    strcat(comp_ip, path);
  }
  for (int i = 0; i < 15; i++) {
    if (comp_ip[i] == '\n') {
      comp_ip[i] = '\0';
    }
  }
  printf("MAINLIB | Found IP to be %s\n", comp_ip);
  pclose(fp);

  int part1, part2, part3, part4;
  part1=part2=part3=part4=0;
  sscanf(comp_ip, "%d.%d.%d.%d", &part1, &part2, &part3, &part4);
  int ip_int = (part1 * pow(256, 3)) + (part2 * pow(256, 2)) + (part3 * 256) + (part4);

  pthread_t server;
  struct server_args * udp_args = malloc(sizeof(struct server_args));
  udp_args->devices = devices;
  udp_args->m = m;
  pthread_create(&server, NULL, start_server, (void *) udp_args);
  printf("MAINLIB | Created server thread.\n");

  for (int i = 0; i < m; i++) {
    pthread_t worker;
    // HACK Trickery with structs as pthread_create only allows one argument to function for some reason.
    struct worker_args * pass_args = malloc(sizeof(struct worker_args));
    pass_args->name = i;
    pass_args->map = map;
    pass_args->reduce = reduce;
    pass_args->length = length;
    pass_args->ip = ip_int;
    pthread_create(&worker, NULL, start_worker, (void *) pass_args);
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
