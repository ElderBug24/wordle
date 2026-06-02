#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#define WORDLEN 5
#define TRIALS 6

int main() {
  char buf[WORDLEN + 2] = {0};
  char word[WORDLEN] = "hello";

  for (size_t t = 0; t < TRIALS;) {
    fgets(buf, WORDLEN + 2, stdin);
    printf("\033[A\033[2K");

    if (strlen(buf) - 1 < WORDLEN) {
      printf("\033[%uC\e[0;31mError: word is too short\e[0m\r", WORDLEN + 1);
      continue;
    } else {
      size_t correct = 0;
      for (int i = 0; i < WORDLEN; ++i) {
        char c = buf[i];
        bool contains = false;

        if (word[i] == c) {
          printf("\033[38;2;%u;%u;%um", 83, 141, 78);
          correct += 1;
        }
        else {
          for (size_t j = 0; j < WORDLEN; ++j) {
            if (i == j) continue;
            if (word[j] == c) {
              contains = true;
              break;
            }
          }

          if (contains) printf("\033[38;2;%u;%u;%um", 181, 159, 59);
        }
        putc(c, stdout);
        printf("\e[0m");
      }
      putc('\n', stdout);

      if (correct == WORDLEN) {
        printf("\nCorrect!\n");
        return 0;
      }
    }

    t += 1;
  }

  return 0;
}

