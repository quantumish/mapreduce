#include "mapreduce.h"

// A simple map function for getting case-insensitive letter frequencies
// by mapping <document, contents> to a list of <letter, count>.
// Takes str_pair <document, contents>, returns int_pair* to array of <letter, count>
struct pair* map (struct pair input_pair)
{
  struct pair * output_pairs = malloc(sizeof(struct pair)*27);
  for (int i = 0; i < 26; i++) {
    char * lower = malloc(sizeof(char)*2);
    char * upper = malloc(sizeof(char)*2);
    sprintf(lower, "%c", i + 97);
    sprintf(upper, "%c", i + 65);
    int* count = malloc(sizeof(int));
    char * tmp = input_pair.value;    // Modified from https://stackoverflow.com/questions/9052490/find-the-count-of-substring-in-string
    while(tmp = strstr(tmp, lower)) { // HACK-ish: The while here is a little strange since it's using result of assignment but I understand
      *(count) = *(count) + 1;        // this shortcut to essentially be checking if strstr(tmp, lower) is NULL or not although I may just want
      tmp++;                          // to switch to a normal-er looking while loop.
    }
    tmp = input_pair.value;
    while (tmp = strstr(tmp, upper)) {
      *(count) = *(count) + 1;
      tmp++;
    }
    struct pair letter = {upper, count};
    output_pairs[i] = letter;
  }
  return output_pairs;
}

// A simple reduce function for reducing a list of <letter, count> to a total count for each letter.
// Takes int_pair* to array containing <letter, count> and returns int_pair* to array with final counts.
// NOTE: Expects a sort of 'null-terminated' array of int_pairs, last int_pair's key must be '\0', returns
// similar type of terminated array.
struct pair * reduce(struct pair* intermediate_pairs)
{
  struct pair * output_pairs = malloc(sizeof(struct pair)*27);
  struct pair * filtered_pairs = malloc(sizeof(struct pair)*27);
  for (int i = 0; i < 26; i++) {
    char * upper = malloc(sizeof(char)*2);
    int * temp = malloc(sizeof(int));
    *temp = 0;
    sprintf(upper, "%c", i + 65);
    struct pair blank = {upper, temp};
    output_pairs[i] = blank;
  }
  /* char ascii[3]; */
  /* sprintf(ascii, "%d", *(char*)(intermediate_pairs[0].key)); */
  /* printf("CHECKPOINT\n"); */
  for (int i = 0; intermediate_pairs[i].key != 0x0 ; i++) {
    char* ascii = malloc(3*sizeof(char));
    char* ascii_num = malloc(3*sizeof(char));
    sprintf(ascii, "%s", (char*)(intermediate_pairs[i].key));
    sprintf(ascii_num, "%d", *ascii);
    output_pairs[strtol(ascii_num, NULL, 10) - 65].key = ascii;
    int* newval = malloc(sizeof(int));
    *newval = *(int*)intermediate_pairs[i].value + *(int*)output_pairs[strtol(ascii_num, NULL, 10) - 65].value;
    output_pairs[strtol(ascii_num, NULL, 10) - 65].value = newval;
  }
  output_pairs[26].key = 0x0;
  output_pairs[26].value = 0x0;
  int i = 0;
  while (output_pairs[i].value != 0x0) {
    filtered_pairs[i].value = output_pairs[i].value;
    filtered_pairs[i].key = output_pairs[i].key;
    i++;
  }
  return filtered_pairs;
}

void translate(char* path)
{
  FILE* rptr = fopen(path, "r");
  FILE* wptr = fopen("./translated", "w");
  char* line = malloc(MAXLINE*sizeof(void*));
  char* newline = malloc(MAXLINE*sizeof(char));
  for (int i = 0; i < 26; i++) {
    fgets(line, MAXLINE, rptr);
    void* addr1;
    void* addr2;
    sscanf(line, "%p %p", &addr1, &addr2);
    sprintf(newline, "%s %i", (char*)addr1, *(int*)addr2);
    fprintf(wptr, "%s %i\n",(char*)addr1, *(int*)addr2);
  }
  fclose(rptr);
  fclose(wptr);
  free(newline);
  free(line);
}

int main(int argc, char** argv)
{
  begin(argv[2], map, reduce, translate, strtol(argv[1], NULL, 10), 26, argv[3], strtol(argv[4], NULL, 10));
}
