#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 5000
#define MAXLINE 2048
#define MSG_CONFIRM 1

int main ()
{
  /* struct hostent *hp; */
  /* struct sockaddr_in servaddr; */
  /* char *my_message = "this is a test message"; */
  /* memset((char*)&servaddr, 0, sizeof(servaddr)); */
  /* servaddr.sin_family = AF_INET; */
  /* servaddr.sin_port = htons(port); */
  /* hp = gethostbyname(host); */
  /* if (!hp) { */
  /*   fprintf(stderr, "could not obtain address of %s\n", host); */
  /*   return 0; */
  /* } */

  /* /\* put the host's address into the server address structure *\/ */
  /* memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length); */

  /* /\* send a message to the server *\/ */
  /* if (sendto(fd, my_message, strlen(my_message), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { */
  /*   perror("sendto failed"); */
  /*   return 0; */
  /* } */
  int sockfd;
  char buffer[MAXLINE];
  char *hello = "Hello from client";
  struct sockaddr_in servaddr;

  // Creating socket file descriptor
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  int n, len;

  sendto(sockfd, (const char *)hello, strlen(hello),
         MSG_CONFIRM, (const struct sockaddr *) &servaddr,
         sizeof(servaddr));
  printf("Hello message sent.\n");

  n = recvfrom(sockfd, (char *)buffer, MAXLINE,
               MSG_WAITALL, (struct sockaddr *) &servaddr,
               &len);
  buffer[n] = '\0';
  printf("Server : %s\n", buffer);

  close(sockfd);
  return 0;
}
