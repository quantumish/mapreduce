#include "mapreduce.h"

// A simple map function for mapping <word, string> to <word, count>.
// strtok code modified from http://www.cplusplus.com/reference/cstring/strtok/
int map(char* word, char* string)
{
  int wordCount = 0;
  char * token;
  token = strtok (string," ");
  while (token != NULL)
  {
    token = strtok (NULL, " ");
    if (strcmp(token, word) == 0)
    {
      wordCount += 1;
    }
  }
  return wordCount;
}

int main()
{
  char word[50];
  strcpy(word, "A");
  char sentence[50];
  strcpy(sentence,"Look, a spider AAAAA");
  map(word, sentence);
}


// A simple reduce function for reducing <word, count> to a total count
// This requires summing the list, which you may notice is inefficient!
// We could use MapReduce inside of here as well, but this is merely
// proof-of-concept.
int reduce(int* wordCounts)
{
  int totalCount = 0;
  for (int i = 0; i <= sizeof(wordCounts)/sizeof(int); i++)
  {
    totalCount += wordCounts[i];
  }
  return totalCount;
}

/* int main() */
/* { */
/*   begin("/Users/davidfreifeld/testing.txt", map, reduce, 8); */
/* } */
