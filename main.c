#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 5000
#define MAXLINE 1024

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

// General reference for the UDP server:
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
// https://www.geeksforgeeks.org/udp-client-server-using-connect-c-implementation/
void server()
{
  bzero(&servaddr, sizeof(servaddr)); 
  listenfd = socket(AF_INET, SOCK_DGRAM, 0);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);
  servaddr.sin_family = AF_INET;
  bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
  len = sizeof(cliaddr);
  int n = recvfrom(listenfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
  buffer[n] = '\0';
  puts(buffer);
  sendto(listenfd, message, MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
}

void begin(char* path, void (*map)(char*), void (*reduce))
{
  split(path, 8);
}


int main(int argc, char** argv)
{
    //printf("Don't do this! Call functions instead.\n");
  return 0;
}
