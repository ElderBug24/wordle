#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#define WORDLEN 5
#define TRIALS 6

int main() {
  char buf[WORDLEN] = {0};
  char word[WORDLEN] = "hello";
  int input;

  for (size_t t = 0; t < TRIALS;) {
    for (size_t i = 0; i < WORDLEN; ++i) {
      input = getchar();
      if (input == EOF) return 1;

      char inputchar = (char)input;
      if (inputchar == '\r') goto break_short_r;
      if (inputchar == '\n') goto break_short;
      buf[i] = inputchar;
    }
    input = getchar();
    if (input == EOF) return 1;
    if ((char)input == '\r') {
      input = getchar();
      if (input == EOF) return 1;
    } else if ((char)input == '\n') {
    } else goto break_long;

    printf("\033[A\033[2K\r");
    goto break_end;

break_short_r:
    input = getchar();
    if (input == EOF) return 1;
    goto break_short;
break_short:
    printf("\033[A\033[2K\r\033[%uC\e[0;31mError: word is too short\e[0m\r", WORDLEN + 1);
    continue;
break_long:
    printf("\033[A\033[2K\r\033[%uC\e[0;31mError: word is too long\e[0m\r", WORDLEN + 1);
    do {
      input = getchar();
      if (input == EOF) return 1;
    } while ((char)input != '\n');
    continue;
break_end:
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

    t += 1;
  }

  return 0;
}

