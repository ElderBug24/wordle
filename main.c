#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>


#ifndef WORDLEN
#define WORDLEN 5
#endif
#ifndef TRIALS
#define TRIALS 6
#endif
#ifndef COLORBACKGROUND
#define COLORBACKGROUND false
#endif
#ifndef FILELA
#define FILELA "wordle-La.txt"
#endif
#ifndef FILETA
#define FILETA "wordle-Ta.txt"
#endif

int main() {
  FILE *file_la = fopen(FILELA, "rb");
  if (!file_la) return 1;
  fseek(file_la, 0, SEEK_END);
  size_t file_la_len = ftell(file_la);
  fseek(file_la, 0, SEEK_SET);
  char *buffer_la = malloc(file_la_len + 1);
  if (!buffer_la) {
    fclose(file_la);
    return 1;
  }
  fread(buffer_la, 1, file_la_len, file_la);
  buffer_la[file_la_len] = 0;
  fclose(file_la);
  bool la_crlf = buffer_la[WORDLEN] == '\r';
  assert((buffer_la[WORDLEN] == '\r' && buffer_la[WORDLEN] == '\n') ^ buffer_la[WORDLEN] == '\n');
  size_t elementlen_la = WORDLEN + 1 + la_crlf;
  size_t count_la = file_la_len / elementlen_la;

  FILE *file_ta = fopen(FILETA, "rb");
  if (!file_ta) return 1;
  fseek(file_ta, 0, SEEK_END);
  size_t file_ta_len = ftell(file_ta);
  fseek(file_ta, 0, SEEK_SET);
  char *buffer_ta = malloc(file_ta_len + 1);
  if (!buffer_ta) {
    fclose(file_ta);
    return 1;
  }
  fread(buffer_ta, 1, file_ta_len, file_ta);
  buffer_ta[file_ta_len] = 0;
  fclose(file_ta);
  bool ta_crlf = buffer_ta[WORDLEN] == '\r';
  assert((buffer_ta[WORDLEN] == '\r' && buffer_ta[WORDLEN] == '\n') ^ buffer_ta[WORDLEN] == '\n');
  size_t elementlen_ta = WORDLEN + 1 + ta_crlf;
  size_t count_ta = file_ta_len / elementlen_ta;

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

