#include "mapreduce.h"

// TODO Make arguments and returns key value pairs instead of just values

// A simple map function for mapping <word, string> to <word, count>.
int map (char* string)
{
  char* word = "a"; // TODO implement Google example where its each word in document
  int count = 0;
  char * tmp = string;
  while(tmp = strstr(tmp, word))
  {
    count++;
    tmp++;
  }
  return count;
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

int main()
{
  begin("/Users/davidfreifeld/testing.txt", map, reduce, 8);
}
