#include "mapreduce.h"

// TODO Make arguments and returns key value pairs instead of just values

// A simple map function for mapping <word, string> to <word, count>.
struct int_pair* map (struct str_pair input_pair)
{
  /* struct int_pair ex = {"/", 0}; */
  /* struct int_pair output_pairs[26] = {ex}; */
  struct int_pair * output_pairs = (struct int_pair *)malloc(sizeof(struct int_pair)*26);
  for (int i = 0; i < 26; i++) {
    char lower[2];
    sprintf(lower, "%c", i + 97);
    char upper[2];
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
    printf("SHOULD BE %s %i\n", upper, count);
    struct int_pair letter = {upper, count};
    output_pairs[i] = letter;
  }
  return output_pairs;
}

/* A simple reduce function for reducing <word, count> to a total count */
/* This requires summing the list, which you may notice is inefficient! */
/* We could use MapReduce inside of here as well, but this is merely */
/* proof-of-concept. */
struct int_pair * reduce(struct int_pair* wordCounts)
{
  /* int totalCount = 0; */
  /* for (int i = 0; i <= sizeof(wordCounts)/sizeof(int); i++) */
  /* { */
  /*   totalCount += wordCounts[i]; */
  /* } */
  /* return totalCount; */
}

int main()
{
  begin("/Users/davidfreifeld/testing.txt", map, reduce, 8, 26);
}
