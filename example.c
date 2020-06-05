#include "mapreduce.h"

// A simple map function for getting case-insensitive letter frequencies
// by mapping <document, contents> to a list of <letter, count>.
// Takes str_pair <document, contents>, returns int_pair* to array of <letter, count>
struct int_pair* map (struct str_pair input_pair)
{
  struct int_pair * output_pairs = malloc(sizeof(struct int_pair)*26);
  for (int i = 0; i < 26; i++) {
    char * lower = malloc(sizeof(char)*2);
    char * upper = malloc(sizeof(char)*2);
    sprintf(lower, "%c", i + 97);
    sprintf(upper, "%c", i + 65);
    int count = 0;
    char * tmp = input_pair.value;    // Modified from https://stackoverflow.com/questions/9052490/find-the-count-of-substring-in-string
    while(tmp = strstr(tmp, lower)) { // HACK-ish: The while here is a little strange since it's using result of assignment but I understand
      count++;                        // this shortcut to essentially be checking if strstr(tmp, lower) is NULL or not although I may just want
      tmp++;                          // to switch to a normal-er looking while loop.
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

// A simple reduce function for reducing a list of <letter, count> to a total count for each letter.
// Takes int_pair* to array containing <letter, count> and returns int_pair* to array with final counts.
// NOTE: Expects a sort of 'null-terminated' array of int_pairs, last int_pair's key must be '\0', returns
// similar type of terminated array.
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
  }
  return filtered_pairs;
}

int main()
{
  begin("./valley.txt", map, reduce, 8, 26);
}
