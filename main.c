#include <stdio.h>
#include <stdlib.h>

void split(char* path, int num_splits)
{
  FILE *rptr;
  rptr = fopen(path, "r");
  if (rptr==NULL) {
    printf("Error! File could not be opened.");
    exit(1);
  }
  char fileoutputname[15];
  char line[128];
  FILE *wptr;
  int filecounter=1;
  int linecounter=1;
  int linecount=1;
  sprintf(fileoutputname, "file_part%d", filecounter);
  wptr = fopen(fileoutputname, "w");
  while (fgets(line, sizeof line, rptr) != NULL) {
    linecount++;
  }
  while (fgets(line, sizeof line, rptr) != NULL) {
    if ((linecounter/linecount) == (1/num_splits)) {
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

void begin(char* path, void (*map)(char*), void (*reduce))
{
  split(path, 8);
}


int main(int argc, char** argv)
{
  split("/Users/davidfreifeld/testing.txt",8);
  //printf("Don't do this! Call functions instead.\n");
  return 0;
}
