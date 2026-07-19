#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#ifdef _WIN32
#include <conio.h>
void enable_raw_mode() {}
void disable_raw_mode() {}
#else
#include <termios.h>
#include <unistd.h>
struct termios orig;
void enable_raw_mode() {
  struct termios raw;
  tcgetattr(STDIN_FILENO, &orig);
  raw = orig;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig);
}
typedef int(*getch_t)(void);
getch_t getch = getchar;
#endif

#ifndef DISPLAYKEYBOARD
#define DISPLAYKEYBOARD true
#endif

#include "wordle-La-compact.h"
#include "wordle-Ta-compact.h"
#include "definitions.h"

#ifndef WORDLEN
#define WORDLEN 5
#endif
#ifndef TRIALS
#define TRIALS 6
#endif
#ifndef WORDDEFINITIONLEN
#define WORDDEFINITIONLEN 64
#endif

#define LETTERSCOUNT ('z' - 'a' + 1)
enum {
  LETTER_UNKNOWN = 0,
  LETTER_RIGHT,
  LETTER_WRONG,
  LETTER_ABSENT
};

#define KEYBOARD_ROW1 "qwertyuiop"
#define KEYBOARD_ROW2 "asdfghjkl"
#define KEYBOARD_ROW3 "zxcvbnm"

#define COLOR_GREEN "83;141;78"
#define COLOR_YELLOW "181;159;59"
#define COLOR_GREY "58;58;60"

char word[WORDLEN];
size_t word_index;

typedef enum {
  QUIT_CORRECT,
  QUIT_ERR_STDIN
} quit_code_e;

void quit(quit_code_e code) {
  switch (code) {
    case QUIT_CORRECT: {
#if DISPLAYKEYBOARD
      printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[3A");
#endif
      printf("\nCorrect!\n%.*s\n", (unsigned int) WORDDEFINITIONLEN, &buffer_definitions[word_index * WORDDEFINITIONLEN]);
      fflush(stdout);
      exit(0);
    }
    case QUIT_ERR_STDIN: {
#if DISPLAYKEYBOARD
      printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[3A");
#endif
      printf("\n\033[0;31mError reading standard input\033[0m\nThe word was '%.*s'\n%.*s\n", (unsigned int) WORDLEN, word, (unsigned int) WORDDEFINITIONLEN, &buffer_definitions[word_index * WORDDEFINITIONLEN]);
      fflush(stdout);
      exit(1);
    }
  }
}

void handler(int sig) {
  (void) sig;
  quit(QUIT_ERR_STDIN);
}

#ifdef _WIN32
enum {
  CHAR_ENTER     =  13,
  CHAR_BACKSPACE =   8,
  CHAR_CTRLC     =   3
};
#else
enum {
  CHAR_ENTER     =  10,
  CHAR_BACKSPACE = 127,
  CHAR_CTRLC     =   3
};
#endif

int main(void) {
  signal(SIGINT, handler);

  size_t count_la = file_la_len / WORDLEN;
  size_t count_ta = file_ta_len / WORDLEN;
  char buf[WORDLEN];
  bool used[WORDLEN];

  srand((unsigned)time(NULL));
  word_index = (size_t) rand() % count_la;

  printf("Welcome to Wordle!\n");
  for (size_t i = 0; i < WORDLEN; ++i) putchar('_');
  putchar('\r');
  uint8_t letters_state[LETTERSCOUNT] = {0};
  memcpy(word, &buffer_la[word_index * WORDLEN], WORDLEN);

  for (size_t t = 0; t < TRIALS;) {
    unsigned char buf_count = 0;
    bool input = true;
    enable_raw_mode();
    while (input) {
      int c = _getch();

      switch (c) {
        case CHAR_CTRLC:
          quit(QUIT_ERR_STDIN);
          break;
        case CHAR_ENTER:
          if (buf_count == WORDLEN) {
            bool valid = false;
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

            if (!valid) {
              printf("\033[?25l\033[2K\r%.*s", buf_count, buf);
              printf("\r\033[%uC\033[0;31mError: word not in the list\033[0m\r", WORDLEN + 1);
              if (buf_count > 0) printf("\033[%uC", buf_count);
              printf("\033[?25h");
              continue;
            } else input = false;
          }
          else {
            printf("\033[?25l\033[2K\r%.*s", buf_count, buf);
            printf("\r\033[%uC\033[0;31mError: word is too short\033[0m\r", WORDLEN + 1);
            if (buf_count > 0) printf("\033[%uC", buf_count);
            printf("\033[?25h");
            break;
          }
          break;
        case CHAR_BACKSPACE:
          if (buf_count > 0) {
            printf("\033[?25l\r");
            if (buf_count > 1) printf("\033[%uC", buf_count - 1);
            printf("_\033[D\033[?25h");
            buf_count -= 1;
          }
          break;
        default:
          if (buf_count < WORDLEN) {
            if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
            else if (c < 'a' || c > 'z') continue;
            printf("\033[?25l\r");
            if (buf_count > 0) printf("\033[%uC", buf_count);
            printf("%c\033[?25h", c);
            buf[buf_count] = (char) c;
            buf_count += 1;
          }
          break;
      }
    }
    disable_raw_mode();

    size_t correct = 0;
    memset(used, 0, WORDLEN);
    for (int i = 0; i < WORDLEN; ++i) {
      if (word[i] == buf[i]) {
        used[i] = true;
      }
    }
    printf("\033[?25l\033[2K\r");
    for (size_t i = 0; i < WORDLEN; ++i) {
      char c = buf[i];
      bool contains = false;

      if (word[i] == c) {
        printf("\033[38;2;" COLOR_GREEN "m");
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
          printf("\033[38;2;" COLOR_YELLOW "m");
          if (letters_state[c - 'a'] == LETTER_UNKNOWN) letters_state[c - 'a'] = LETTER_WRONG;
        } else {
          if (letters_state[c - 'a'] == LETTER_UNKNOWN) letters_state[c - 'a'] = LETTER_ABSENT;
        }
      }
      putchar(c);
      printf("\033[0m");
    }
    printf("\n\033[?25h");

#if DISPLAYKEYBOARD
    printf("\n\033[2K");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW1); ++i) {
      char c = KEYBOARD_ROW1[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          printf("\033[38;2;" COLOR_GREEN "m");
          break;
        case LETTER_WRONG:
          printf("\033[38;2;" COLOR_YELLOW "m");
          break;
        case LETTER_ABSENT:
          printf("\033[38;2;" COLOR_GREY "m");
          break;
      }
      putchar(c);
      printf("\033[0m");
    }
    printf("\n\033[2K");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW2); ++i) {
      char c = KEYBOARD_ROW2[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          printf("\033[38;2;" COLOR_GREEN "m");
          break;
        case LETTER_WRONG:
          printf("\033[38;2;" COLOR_YELLOW "m");
          break;
        case LETTER_ABSENT:
          printf("\033[38;2;" COLOR_GREY "m");
          break;
      }
      putchar(c);
      printf("\033[0m");
    }
    printf("\n\033[2K ");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW3); ++i) {
      char c = KEYBOARD_ROW3[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          printf("\033[38;2;" COLOR_GREEN "m");
          break;
        case LETTER_WRONG:
          printf("\033[38;2;" COLOR_YELLOW "m");
          break;
        case LETTER_ABSENT:
          printf("\033[38;2;" COLOR_GREY "m");
          break;
      }
      putchar(c);
      printf("\033[0m");
    }
    putchar('\n');

    printf("\033[4A\033[2K");
#endif

    if (correct == WORDLEN) {
      quit(QUIT_CORRECT);
    }

    t += 1;
  }

  printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[2A");
  printf("The word was '%.*s'\n", (unsigned int) WORDLEN, word);
  printf("%.*s\n", (unsigned int) WORDDEFINITIONLEN, &buffer_definitions[word_index * WORDDEFINITIONLEN]);

  fflush(stdout);
  return 0;
}

