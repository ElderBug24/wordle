#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#ifndef WORDLEN
#define WORDLEN 5
#endif
#ifndef TRIALS
#define TRIALS 6
#endif
#ifndef COLORBACKGROUND
#define COLORBACKGROUND false
#endif

int main() {
  printf("Welcome to Wordle!\n");
  for (size_t i = 0; i < WORDLEN; ++i) putc('_', stdout);
  putc('\r', stdout);

  char buf[WORDLEN] = {0};
  char word[WORDLEN] = "table";
  bool used[WORDLEN];
  int input;

  for (size_t t = 0; t < TRIALS;) {
    bool invalid_char = false;
    for (size_t i = 0; i < WORDLEN; ++i) {
      input = getchar();
      if (input == EOF) return 1;

      char inputchar = (char)input;
      if (inputchar == '\r') goto break_short_r;
      if (inputchar == '\n') goto break_short;
      if (!((inputchar >= 'A' && inputchar <= 'Z') || (inputchar >= 'a' && inputchar <= 'z'))) invalid_char = true;
      else if (inputchar >= 'A' && inputchar <= 'Z') inputchar += ('a' - 'A');

      buf[i] = inputchar;
    }
    input = getchar();
    if (input == EOF) return 1;
    if ((char)input == '\r') {
      input = getchar();
      if (input == EOF) return 1;
    } else if ((char)input == '\n') {
    } else goto break_long;
    if (invalid_char) goto break_invalid_char;

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
break_invalid_char:
    printf("\033[A\033[2K\r\033[%uC\e[0;31mError: word contains invalid characters: only alphabetic characters are allowed (uppercase letters get normalized)\e[0m\r", WORDLEN + 1);
    continue;
break_end:
    printf("\033[A\033[2K\r");
    size_t correct = 0;
    memset(used, 0, WORDLEN);
    for (int i = 0; i < WORDLEN; ++i) {
      if (word[i] == buf[i]) {
        used[i] = true;
      }
    }
    for (int i = 0; i < WORDLEN; ++i) {
      char c = buf[i];
      bool contains = false;

      if (word[i] == c) {
        printf("\033[%c8;2;%u;%u;%um", COLORBACKGROUND ? '4' : '3', 83, 141, 78);
        correct += 1;
      }
      else {
        for (size_t j = 0; j < WORDLEN; ++j) {
          if (i == j) continue;
          if (word[j] == c && !used[j]) {
            contains = true;
            used[j] = true;
            break;
          }
        }

        if (contains) printf("\033[%c8;2;%u;%u;%um", COLORBACKGROUND ? '4' : '3', 181, 159, 59);
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

