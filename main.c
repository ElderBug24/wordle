#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#ifndef DISPLAYKEYBOARD
#define DISPLAYKEYBOARD true
#endif

#include "wordle-La-compact.h"
#include "wordle-Ta-compact.h"

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

#define LETTERSCOUNT ('z' - 'a' + 1)
#define LETTER_UNKNOWN 0
#define LETTER_RIGHT 1
#define LETTER_WRONG 2
#define LETTER_ABSENT 3

#define KEYBOARD_ROW1 "qwertyuiop"
#define KEYBOARD_ROW2 "asdfghjkl"
#define KEYBOARD_ROW3 "zxcvbnm"

#define COLOR_GREEN "83;141;78"
#define COLOR_YELLOW "181;159;59"
#define COLOR_GREY "58;58;60"

int main() {
  size_t file_la_len = wordle_La_compact_txt_len;
  char* buffer_la = wordle_La_compact_txt;
  size_t file_ta_len = wordle_Ta_compact_txt_len;
  char* buffer_ta = wordle_Ta_compact_txt;

  size_t count_la = file_la_len / WORDLEN;
  size_t count_ta = file_ta_len / WORDLEN;

  srand((unsigned)time(NULL));
  size_t x;
  size_t limit = (size_t) -1 - ((size_t) -1 % (count_la + 1));
  do {
    x = rand();
    x = (x << 16) ^ rand();
  } while (x > limit);
  size_t word_index = x % (count_la + 1);

  printf("Welcome to Wordle!\n");
  for (size_t i = 0; i < WORDLEN; ++i) putc('_', stdout);
  putc('\r', stdout);

  char buf[WORDLEN] = {0};
  char word[WORDLEN];
  bool used[WORDLEN];
  int input;
  uint8_t letters_state[LETTERSCOUNT];

  memcpy(word, &buffer_la[word_index * WORDLEN], WORDLEN);
  memset(letters_state, LETTER_UNKNOWN, LETTERSCOUNT);

  for (size_t t = 0; t < TRIALS;) {
    bool invalid_char = false;
    for (size_t i = 0; i < WORDLEN; ++i) {
      input = getchar();
      if (input == EOF) goto exit_error_stdin;

      char inputchar = (char)input;
      if (inputchar == '\r') goto break_short_r;
      if (inputchar == '\n') goto break_short;
      if (!((inputchar >= 'A' && inputchar <= 'Z') || (inputchar >= 'a' && inputchar <= 'z'))) invalid_char = true;
      else if (inputchar >= 'A' && inputchar <= 'Z') inputchar += ('a' - 'A');

      buf[i] = inputchar;
    }
    input = getchar();
    if (input == EOF) goto exit_error_stdin;
    if ((char)input == '\r') {
      input = getchar();
      if (input == EOF) goto exit_error_stdin;
    } else if ((char)input == '\n') {
    } else goto break_long;
    if (invalid_char) goto break_invalid_char;

    bool valid = false;
#ifdef NOBINARYSEARCH
    for (size_t i = 0; i < count_la; ++i) {
      if (memcmp(buf, &buffer_la[i * elementlen_la], WORDLEN) == 0) {
        valid = true;
        break;
      }
    }
    if (!valid) {
      for (size_t i = 0; i < count_ta; ++i) {
        if (memcmp(buf, &buffer_ta[i * elementlen_la], WORDLEN) == 0) {
          valid = true;
          break;
        }
      }
    }
#else
    size_t min = 0;
    size_t max = count_la;
    while (min < max) {
      size_t index = min + (max - min) / 2;
      int cmp = memcmp(buf, &buffer_la[index * WORDLEN], WORDLEN);
      if (cmp == 0) {
        valid = true;
        break;
      } else if (cmp < 0) {
        max = index;
      } else {
        min = index + 1;
      }
    }
    min = 0;
    max = count_ta;
    if (!valid) {
      while (min < max) {
        size_t index = min + (max - min) / 2;
        int cmp = memcmp(buf, &buffer_ta[index * WORDLEN], WORDLEN);
        if (cmp == 0) {
          valid = true;
          break;
        } else if (cmp < 0) {
          max = index;
        } else {
          min = index + 1;
        }
      }
    }
#endif

    if (!valid) {
      printf("\033[A\033[2K\r\033[%uC\033[0;31mError: word not in the list\033[0m\r", WORDLEN + 1);
      continue;
    }

    goto break_end;

break_short_r:
    input = getchar();
    if (input == EOF) goto exit_error_stdin;
    goto break_short;
break_short:
    printf("\033[A\033[2K\r\033[%uC\033[0;31mError: word is too short\033[0m\r", WORDLEN + 1);
    continue;
break_long:
    printf("\033[A\033[2K\r\033[%uC\033[0;31mError: word is too long\033[0m\r", WORDLEN + 1);
    do {
      input = getchar();
      if (input == EOF) goto exit_error_stdin;
    } while ((char)input != '\n');
    continue;
break_invalid_char:
    printf("\033[A\033[2K\r\033[%uC\033[0;31mError: word contains invalid characters: only alphabetic characters are allowed (uppercase letters get normalized)\033[0m\r", WORDLEN + 1);
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
        printf("\033[%c8;2;" COLOR_GREEN "m", COLORBACKGROUND ? '4' : '3');
        correct += 1;
        letters_state[c - 'a'] = LETTER_RIGHT;
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

        if (contains) {
          printf("\033[%c8;2;" COLOR_YELLOW "m", COLORBACKGROUND ? '4' : '3');
          if (letters_state[c - 'a'] == LETTER_UNKNOWN) letters_state[c - 'a'] = LETTER_WRONG;
        } else {
          if (letters_state[c - 'a'] == LETTER_UNKNOWN) letters_state[c - 'a'] = LETTER_ABSENT;
        }
      }
      putc(c, stdout);
      printf("\033[0m");
    }
    putc('\n', stdout);

#if DISPLAYKEYBOARD
    printf("\n\033[2K");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW1); ++i) {
      char c = KEYBOARD_ROW1[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          printf("\033[%c8;2;" COLOR_GREEN "m", COLORBACKGROUND ? '4' : '3');
          break;
        case LETTER_WRONG:
          printf("\033[%c8;2;" COLOR_YELLOW "m", COLORBACKGROUND ? '4' : '3');
          break;
        case LETTER_ABSENT:
          printf("\033[%c8;2;" COLOR_GREY "m", COLORBACKGROUND ? '4' : '3');
          break;
      }
      putc(c, stdout);
      printf("\033[0m");
    }
    printf("\n\033[2K");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW2); ++i) {
      char c = KEYBOARD_ROW2[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          printf("\033[%c8;2;" COLOR_GREEN "m", COLORBACKGROUND ? '4' : '3');
          break;
        case LETTER_WRONG:
          printf("\033[%c8;2;" COLOR_YELLOW "m", COLORBACKGROUND ? '4' : '3');
          break;
        case LETTER_ABSENT:
          printf("\033[%c8;2;" COLOR_GREY "m", COLORBACKGROUND ? '4' : '3');
          break;
      }
      putc(c, stdout);
      printf("\033[0m");
    }
    printf("\n\033[2K ");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW3); ++i) {
      char c = KEYBOARD_ROW3[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          printf("\033[%c8;2;" COLOR_GREEN "m", COLORBACKGROUND ? '4' : '3');
          break;
        case LETTER_WRONG:
          printf("\033[%c8;2;" COLOR_YELLOW "m", COLORBACKGROUND ? '4' : '3');
          break;
        case LETTER_ABSENT:
          printf("\033[%c8;2;" COLOR_GREY "m", COLORBACKGROUND ? '4' : '3');
          break;
      }
      putc(c, stdout);
      printf("\033[0m");
    }
    putc('\n', stdout);

    printf("\033[4A\033[2K");
#endif

    if (correct == WORDLEN) {
#if DISPLAYKEYBOARD
      printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[3A");
#endif
      printf("\nCorrect!\n");
      return 0;
    }

    t += 1;
  }

  printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[2A");
  printf("The word was '%.*s'\n", (unsigned int) WORDLEN, word);

#ifdef TERMSAFETY
  printf("Press enter to exit...");
  (void)getchar();
#endif

  fflush(stdout);
  return 0;

exit_error_file:
#if DISPLAYKEYBOARD
  printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[3A");
#endif
  printf("\n\033[0;31mError opening a file\033[0m\n");
  fflush(stdout);
  return 1;
exit_error_allocation:
#if DISPLAYKEYBOARD
  printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[3A");
#endif
  printf("\n\033[0;31mError allocating memory\033[0m\n");
  fflush(stdout);
  return 1;
exit_error_stdin:
#if DISPLAYKEYBOARD
  printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[3A");
#endif
  printf("\n\033[0;31mError reading standard input\033[0m\nThe word was '%.*s'\n", (unsigned int) WORDLEN, word);
  fflush(stdout);
  return 1;
}

