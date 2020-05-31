#include "mapreduce.h"

// TODO Make arguments and returns key value pairs instead of just values

// A simple map function for mapping <word, string> to <word, count>.
struct int_pair* map (struct str_pair inputPair)
{
  struct int_pair list[26];
  for (int i = 0; i < 26; i++)
  {
    char lower = i + 41;
    char upper = i + 61;
    int count = 0;
    char * tmp = inputPair.value;
    struct int_pair test[26];
    while(tmp = strstr(tmp, lower))
    {
      count++;
      tmp++;
    }
    tmp = inputPair.value;
    while (tmp = strstr(tmp, upper)) // tried to do this at same time as last loop, compiler complained about assignment
    {
      count++;
      tmp++;
    }
    list[i].value = count;
    list[i].key = &upper;
  }
  return list;
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
