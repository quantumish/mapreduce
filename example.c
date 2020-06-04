#include "mapreduce.h"

// TODO Make arguments and returns key value pairs instead of just values

// A simple map function for mapping <word, string> to <word, count>.
struct int_pair* map (struct str_pair input_pair)
{
  /* struct int_pair ex = {"/", 0}; */
  /* struct int_pair output_pairs[26] = {ex}; */
  struct int_pair * output_pairs = malloc(sizeof(struct int_pair)*26);
  for (int i = 0; i < 26; i++) {
    char * lower = malloc(sizeof(char)*2);
    char * upper = malloc(sizeof(char)*2);
    sprintf(lower, "%c", i + 97);
    sprintf(upper, "%c", i + 65);
    int count = 0;
    char * tmp = input_pair.value;
    while(tmp = strstr(tmp, lower)) {
      count++;
      tmp++;
    }
    tmp = input_pair.value;
    while (tmp = strstr(tmp, upper)) {
      count++;
      tmp++;
    }
    if (i < 10) {
      printf("%i %s in %s", count, upper, input_pair.value);
    }
    struct int_pair letter = {upper, count};
    output_pairs[i] = letter;
  }
  return output_pairs;
}

/* A simple reduce function for reducing <word, count> to a total count */
/* This requires summing the list, which you may notice is inefficient! */
/* We could use MapReduce inside of here as well, but this is merely */
/* proof-of-concept. */
struct int_pair * reduce(struct int_pair* intermediate_pairs)
{
  struct int_pair * output_pairs = malloc(sizeof(struct int_pair)*27);
  struct int_pair * filtered_pairs = malloc(sizeof(struct int_pair)*27);
  for (int i = 0; i < 26; i++) {
    char * upper = malloc(sizeof(char)*2);
    sprintf(upper, "%c", i + 65);
    struct int_pair blank = {upper, 0};
    output_pairs[i] = blank;
  }
  int i = 0;
  char ascii[3];
  sprintf(ascii, "%d", *(intermediate_pairs[0].key));
  for (int i = 0; intermediate_pairs[i].key != NULL; i++) {
    char ascii[3];
    sprintf(ascii, "%d", *(intermediate_pairs[i].key));
    output_pairs[strtol(ascii, NULL, 10) - 65].value += intermediate_pairs[i].value;
  }
  output_pairs[26].key = '\0';
  output_pairs[26].value = -1;
  int j = 0;
  for (int i = 0; i < 27; i++) {
    if (output_pairs[i].value != 0) {
      filtered_pairs[j].value = output_pairs[i].value;
      filtered_pairs[j].key = output_pairs[i].key;
      j++;
    }
    /* printf("%s:%i vs %s:%i\n",output_pairs[i].key, output_pairs[i].value, filtered_pairs[i].key,filtered_pairs[i].value); */
  }
  return filtered_pairs;
}

int main()
{
  begin("./testing.txt", map, reduce, 8, 26);
}
