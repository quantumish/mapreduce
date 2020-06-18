#include "mapreduce.h"

// A simple map function for getting case-insensitive letter frequencies
// by mapping <document, contents> to a list of <letter, count>.
// Takes str_pair <document, contents>, returns int_pair* to array of <letter, count>
struct pair* map (struct pair input_pair)
{
  struct pair * output_pairs = malloc(sizeof(struct pair)*26);
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
    sprintf(upper, "%c", i + 65);
    struct pair blank = {upper, 0};
    output_pairs[i] = blank;
  }
  /* char ascii[3]; */
  /* sprintf(ascii, "%d", *(char*)(intermediate_pairs[0].key)); */
  /* printf("CHECKPOINT\n"); */
  for (int i = 0; i < 26; i++) {
    char ascii[3];
    /* printf("%s %d <-\n", (char*)(intermediate_pairs[i].key), *(int*)intermediate_pairs[i].value); */
    sprintf(ascii, "%d", *(char*)(intermediate_pairs[i].key));
    output_pairs[strtol(ascii, NULL, 10) - 65].value += *(int*)intermediate_pairs[i].value;
  }
  output_pairs[26].key = '\0';
  int temp = -1;
  output_pairs[26].value = &temp;
  int j = 0;
  for (int i = 0; i < 27; i++) {
    if (output_pairs[i].value != 0) {
      filtered_pairs[j].value = &output_pairs[i].value;
      filtered_pairs[j].key = &output_pairs[i].key;

      j++;
    }
  }
  return filtered_pairs;
}

void translate(char* path)
{
  FILE* rptr = fopen(path, "r");
  FILE* wptr = fopen("./translated", "r");
  char* line = malloc(MAXLINE*sizeof(void*));
  char* newline = malloc(MAXLINE*sizeof(char));
  for (int i = 0; i < 26; i++) {
    fgets(line, MAXLINE, rptr);
    void* addr1;
    void* addr2;
    sscanf(line, "%p %p", &addr1, &addr2);
    printf("%s\n", line);
    sprintf(newline, "%s %i", (char*)addr1, *(int*)addr2);
    fwrite(newline, sizeof(char), MAXLINE*sizeof(char), wptr);
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
