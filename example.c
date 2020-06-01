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
  struct int_pair * output_pairs = malloc(sizeof(struct int_pair)*26);
  for (int i = 0; i < 26; i++) {
    char * upper = malloc(sizeof(char)*2);
    sprintf(upper, "%c", i + 65);
    struct int_pair blank = {upper, 0};
    output_pairs[i] = blank;
  }
  for (int i = 0; (unsigned long) i <= sizeof(intermediate_pairs)/sizeof(struct int_pair); i++) {
    char ascii[3];
    sprintf(ascii, "%d", intermediate_pairs[i].key);
    output_pairs[(int) ascii].value += intermediate_pairs[i].value;
  }
  return output_pairs;
}

int main()
{
  begin("/Users/davidfreifeld/testing.txt", map, reduce, 8, 26);
}
