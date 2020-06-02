#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include "worker.h"

// FIXME DESTROY ME I SHOULD NOT EXIST CAST ME INTO THE FLAMES I AM A SYMPTOM OF TERRIBLE CODE
// Used from https://stackoverflow.com/questions/26620388/c-substrings-c-string-slicing
static void slice_string(char * str, char * buffer, size_t start, size_t end)
{
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

static char* substr(const char *src, int m, int n)
{
    int len = n - m;
    char *dest = malloc(sizeof(char) * (len + 1));
    for (int i = m; i < n && (*(src + i) != '\0'); i++) {
        *dest = *(src + i);
        dest++;
    }
    *dest = '\0';
    return dest - len;
}

/* static char** split_str(char* source, char delimiter) */
/* { */
/*   char * pieces = malloc(100*sizeof(char)); */
/*   /\* char* pieces[100]; // 100-argument outputs are a bit of a no-go so should be fine *\/ */
/*   char* piece; */
/*   piece = strtok (source, &delimiter); */
/*   int i = 0; */
/*   while (piece != NULL) */
/*   { */
/*     pieces[i] = piece; */
/*     piece = strtok (NULL, &delimiter); */
/*     i++; */
/*   } */
/*   return pieces; */
/* } */

static int length_of_file(char* path)
{
  FILE* fp = fopen(path,"r");
  char line[1024];
  int linecount = 0;
  while (fgets(line, sizeof line, fp) != NULL) {
    linecount++;
  }
  return linecount;
}

static void set_output_file(char* path, struct int_pair * pair_list, int length)
{
  FILE * wptr;
  wptr = fopen(path, "w");
  for (int i = 0; i < length; i++) {
    fprintf(wptr, "%s %i\n", pair_list[i].key, pair_list[i].value);
  }
  fclose(wptr);
}

static struct int_pair get_output_file(FILE* fp)
{
  struct int_pair * ret = malloc(sizeof(struct int_pair));
  ret->key = malloc(sizeof(char) * MAXLINE);
  char key[10];
  int val = 0;
  fscanf(fp, "%s%d", key, &val);
  ret->key = key;
  ret->value = val;
  return *ret;
}

/* static struct int_pair * get_output_file(char* path) */
/* { */
/*   FILE* fp = fopen(path,"r"); */
/*   char line[1024]; */
/*   int linecount = 0; */
/*   printf("AA\n"); */
/*   while (fgets(line, sizeof line, fp) != NULL) { */
/*     linecount++; */
/*   } */
/*   rewind(fp); */

/*   struct int_pair * pair_list = malloc(sizeof(struct int_pair)*linecount); */
/*   char** split_line = malloc(MAXLINE*sizeof(char)); */
/*   int i = 0; */
/*   for (int i = 0; i < linecount; i++) { */
/*     pair_list[i].key = " "; */
/*     pair_list[i].value = 0; */
/*   } */
/*   i = 0; */
/*   while (fgets(line, sizeof(line), fp) != NULL) { */
/*     split_line = split_str(line, ' '); */
/*     pair_list[i].key = split_line[0]; */
/*     pair_list[i].value = strtol(split_line[1], NULL, 10); */
/*     printf("WROTE %s %i\n", pair_list[i].key, pair_list[i].value); */
/*     i++; */
/*   } */
/*   printf("TRIPLE CHECK WROTE %s %i\n", pair_list[0].key, pair_list[0].value); */

/*   fclose(fp); */
/*   return pair_list; */
/* } */

// This tutorial helped quite a bit in debugging what was going wrong with connection
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

void* startWorker(void* arguments)
{
  struct args *function_args = (struct args *)arguments;
  char* start = "Online.";

  // Same socket is needed on client end so initialize all over again.
  int s;
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("\n Error : Socket Failed \n");
  }
  struct sockaddr_in addr;
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; // Specify address family.
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY just 0.0.0.0, machine IP address
  addr.sin_port = htons(PORT); // Specify port.

  // Connect to server
  if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("\n Error : Connect Failed \n");
  }
  // NOTE This can and will not work if flag argument set to 1
  sendto(s, start, BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
  printf("Worker%i | Informed server of existence.\n", function_args->name);
  char buf[BUFSIZE];
  int recvlen;
  socklen_t len = sizeof(addr);

  while (1==1) {
    recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &len);

    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("Worker%i | Received %d-byte message from server: \"%s\"\n", function_args->name, recvlen, buf);
      if (strcmp(buf, "Ping")==0) {
        sendto(s, "Pong", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
      }
      char direction[7];
      char args[BUFSIZE];
      // FIXME There should not be two separate functions for slicing a string that only work in certain circumstances. Write your own.
      strcpy(direction, substr(buf, 0, 6));
      slice_string(buf, args, 6, recvlen-1); /* strcpy(args, substr(buf, 6, recvlen)); */
      /* printf("%s\n", args); */
      if (strcmp(direction, "Map---")==0) {
        printf("Worker%i | Starting map of %s.\n", function_args->name, args);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));

        // This was miserable. Strings are bothersome enough in C but files take it to a whole new level. Vague segfaults, bus errors, and more.
        // TODO Make this size dynamic/configurable. Also change to relative path. Also this maybe shouldn't be done worker-side.

        char content[4096]; 
        char line[1024];
        char path[100] = "/Users/davidfreifeld/projects/mapreduce/file_part";
        char* finalpath = strcat(path, args);
        FILE* fp = fopen(finalpath,"r");
        while (fgets(line, sizeof line, fp) != NULL) {
          strcat(content, line);
        }
        fclose(fp);

        struct str_pair file = {finalpath, content};
        struct int_pair * results = (struct int_pair *)malloc(sizeof(struct int_pair)*function_args->length);
        results = (*function_args->map)(file);

        char wpath[100] = "/Users/davidfreifeld/projects/mapreduce/intermediate";
        char s_name[10];
        sprintf(s_name, "%d", function_args->name);
        strcat(wpath, s_name);
        set_output_file(wpath, results, function_args->length);
        printf("Worker%i | Finished writing to file.\n", function_args->name);
        sendto(s, "Done.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));
        printf("Sent message\n");
      }
      else if (strcmp(direction, "Reduce") == 0) {
        printf("Worker%i | Starting reduce of %s.\n", function_args->name, args);
        sendto(s, "Starting.", BUFSIZE, 0, (struct sockaddr*)NULL, sizeof(addr));

        /* char** split_args = split_str(args, '_'); */
        char path1[100] = "/Users/davidfreifeld/projects/mapreduce/intermediate0";
        /* strcat(path1, split_args[0]); */
        struct int_pair result = get_output_file(fopen(path1, "r"));
        printf("%s %i\n", result.key, result.value);
        /* struct int_pair * file1_list = get_output_file(path1); */
        /* for (int i = 0; i < 26; i++) { */
        /*   printf("READ %s %i\n", file1_list[i].key, file1_list[i].value); */
        /* } */

        /* char path2[100] = "/Users/davidfreifeld/projects/mapreduce/intermediate"; */
        /* strcat(path2, split_args[1]); */
        /* struct int_pair * file2_list = get_output_file(path2); */
      }
    }
  }
}
