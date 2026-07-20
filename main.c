#include <signal.h>
#include <stdbool.h>
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
  raw.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig);
}
#define _getch getchar
#endif

#ifndef DISPLAYKEYBOARD
#define DISPLAYKEYBOARD true
#endif

#include "wordle-list-compact.h"
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

static inline void set_color_green(void) { printf("\033[38;2;83;141;78m"); }
static inline void set_color_yellow(void) { printf("\033[38;2;181;159;59m"); }
static inline void set_color_grey(void) { printf("\033[38;2;58;58;60m"); }
static inline void hide_cursor(void) { printf("\033[?25l"); }
static inline void show_cursor(void) { printf("\033[?25h"); }
static inline void reset_styles(void) { printf("\033[0m"); }

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
      printf("\n\033[0;31mError reading standard input");
      reset_styles();
      printf("\nThe word was '%.*s'\n%.*s\n", (unsigned int) WORDLEN, word, (unsigned int) WORDDEFINITIONLEN, &buffer_definitions[word_index * WORDDEFINITIONLEN]);
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
  CHAR_TAB       =   9,
  CHAR_CTRLC     =   3
};
#else
enum {
  CHAR_ENTER     =  10,
  CHAR_BACKSPACE = 127,
  CHAR_TAB       =   9,
  CHAR_CTRLC     =   3
};
#endif

int main(void) {
  signal(SIGINT, handler);

  char buf[WORDLEN];
  bool used[WORDLEN];

  srand((unsigned)time(NULL));
  word_index = wordle_valid_indices[(size_t) rand() % wordle_valid_indices_count];

  printf("Welcome to Wordle!\n");
  unsigned char letters_state[LETTERSCOUNT] = {0};
  memcpy(word, &wordle_buffer[word_index * WORDLEN], WORDLEN);

  for (size_t t = 0; t < TRIALS; ++t) {
    for (unsigned char i = 0; i < WORDLEN; ++i) putchar('_');
    putchar('\r');
    unsigned char buf_count = 0;
    unsigned char buf_cursor = 0;
    bool input = true;
    enable_raw_mode();
    while (input) {
      int c = _getch();

      switch (c) {
        case 0:
        case 224:
          switch (_getch()) {
            case 75:
              goto case_left;
            case 77:
              goto case_right;
            case 83:
              goto case_del;
          }
          break;
        case 27:
          switch (_getch()) {
            case 91:
            case 79:
              switch (_getch()) {
                case 51:
                  switch (_getch()) {
                    case 126:
                      goto case_del;
                  }
                  break;
                case 68:
                  goto case_left;
                case 67:
                  goto case_right;
              }
          }
          break;
        case CHAR_CTRLC:
          quit(QUIT_ERR_STDIN);
          break;
        case CHAR_ENTER:
          if (buf_count == WORDLEN) {
            bool valid = false;
            size_t min = 0;
            size_t max = wordle_buffer_count;
            while (min < max) {
              size_t index = min + (max - min) / 2;
              int cmp = memcmp(buf, &wordle_buffer[index * WORDLEN], WORDLEN);
              if (cmp == 0) {
                valid = true;
                break;
              } else if (cmp < 0) {
                max = index;
              } else {
                min = index + 1;
              }
            }

            if (!valid) {
              hide_cursor();
              printf("\033[2K\r");
              for (unsigned char i = 0; i < WORDLEN; ++i) putchar('_');
              putchar('\r');
              for (unsigned char i = 0; i < buf_count; ++i) {
                if (letters_state[buf[i] - 'a'] == LETTER_ABSENT)
                  set_color_grey();
                putchar(buf[i]);
                reset_styles();
              }
              printf("\r\033[%uC\033[0;31mError: word not in the list\r", WORDLEN + 1);
              reset_styles();
              if (buf_cursor > 0) printf("\033[%uC", buf_cursor);
              show_cursor();
              continue;
            } else input = false;
          }
          else {
            hide_cursor();
            printf("\033[2K\r");
            for (unsigned char i = 0; i < WORDLEN; ++i) putchar('_');
            putchar('\r');
            for (unsigned char i = 0; i < buf_count; ++i) {
              if (letters_state[buf[i] - 'a'] == LETTER_ABSENT)
                set_color_grey();
              putchar(buf[i]);
              reset_styles();
            }
            printf("\r\033[%uC\033[0;31mError: word is too short\r", WORDLEN + 1);
            reset_styles();
            if (buf_cursor > 0) printf("\033[%uC", buf_cursor);
            show_cursor();
            break;
          }
          break;
        case CHAR_BACKSPACE:
          if (buf_cursor > 0) {
            memmove(buf + buf_cursor - 1, buf + buf_cursor, buf_count - buf_cursor);
            buf_count -= 1;
            buf_cursor -= 1;
            hide_cursor();
            printf("\033[2K\r");
            for (unsigned char i = 0; i < WORDLEN; ++i) putchar('_');
            putchar('\r');
            for (unsigned char i = 0; i < buf_count; ++i) {
              if (letters_state[buf[i] - 'a'] == LETTER_ABSENT || buf[i] == '_')
                set_color_grey();
              putchar(buf[i]);
              reset_styles();
            }
            putchar('\r');
            if (buf_cursor > 0) printf("\033[%uC", buf_cursor);
            show_cursor();
          }
          break;
        default:
          if (c == ' ') c = '_';
          if (buf_count < WORDLEN) {
            if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
            else if ((c < 'a' || c > 'z') && c != '_') continue;
            memmove(buf + buf_cursor + 1, buf + buf_cursor, buf_count - buf_cursor);
            buf[buf_cursor] = (char) c;
            buf_count += 1;
            buf_cursor += 1;
            hide_cursor();
            printf("\033[2K\r");
            for (unsigned char i = 0; i < WORDLEN; ++i) putchar('_');
            putchar('\r');
            for (unsigned char i = 0; i < buf_count; ++i) {
              if (letters_state[buf[i] - 'a'] == LETTER_ABSENT || buf[i] == '_')
                set_color_grey();
              putchar(buf[i]);
              reset_styles();
            }
            putchar('\r');
            if (buf_cursor > 0) printf("\033[%uC", buf_cursor);
            show_cursor();
          }
          break;
      }
      goto case_exit;
      case_left:
        if (buf_cursor > 0) {
          buf_cursor -= 1;
          printf("\033[D");
        }
        continue;
      case_right:
        if (buf_cursor < buf_count && buf_cursor < WORDLEN) {
          buf_cursor += 1;
          printf("\033[C");
        }
        continue;
      case_del:
        if (buf_cursor < buf_count) {
          memmove(buf + buf_cursor, buf + buf_cursor + 1, buf_count - buf_cursor);
          buf_count -= 1;
          hide_cursor();
          printf("\033[2K\r");
          for (unsigned char i = 0; i < WORDLEN; ++i) putchar('_');
          putchar('\r');
          for (unsigned char i = 0; i < buf_count; ++i) {
            if (letters_state[buf[i] - 'a'] == LETTER_ABSENT || buf[i] == '_')
              set_color_grey();
            putchar(buf[i]);
            reset_styles();
          }
          putchar('\r');
          if (buf_cursor > 0) printf("\033[%uC", buf_cursor);
          show_cursor();
        }
        continue;
      case_exit:
        continue;
    }
    disable_raw_mode();

    size_t correct = 0;
    memset(used, 0, WORDLEN);
    for (int i = 0; i < WORDLEN; ++i) {
      if (word[i] == buf[i]) {
        used[i] = true;
      }
    }
    hide_cursor();
    printf("\033[2K\r");
    for (size_t i = 0; i < WORDLEN; ++i) {
      char c = buf[i];
      bool contains = false;

      if (word[i] == c) {
        set_color_green();
        correct += 1;
        letters_state[c - 'a'] = LETTER_RIGHT;
      } else {
        for (size_t j = 0; j < WORDLEN; ++j) {
          if (i == j) continue;
          if (word[j] == c && !used[j]) {
            contains = true;
            used[j] = true;
            break;
          }
        }

        if (contains) {
          set_color_yellow();
          if (letters_state[c - 'a'] == LETTER_UNKNOWN) letters_state[c - 'a'] = LETTER_WRONG;
        } else {
          if (letters_state[c - 'a'] == LETTER_UNKNOWN) letters_state[c - 'a'] = LETTER_ABSENT;
        }
      }
      putchar(c);
      reset_styles();
    }
    printf("\n");
    show_cursor();

#if DISPLAYKEYBOARD
    hide_cursor();
    printf("\n\033[2K");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW1); ++i) {
      char c = KEYBOARD_ROW1[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          set_color_green();
          break;
        case LETTER_WRONG:
          set_color_yellow();
          break;
        case LETTER_ABSENT:
          set_color_grey();
          break;
      }
      putchar(c);
      reset_styles();
    }
    printf("\n\033[2K");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW2); ++i) {
      char c = KEYBOARD_ROW2[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          set_color_green();
          break;
        case LETTER_WRONG:
          set_color_yellow();
          break;
        case LETTER_ABSENT:
          set_color_grey();
          break;
      }
      putchar(c);
      reset_styles();
    }
    printf("\n\033[2K ");
    for (size_t i = 0; i < strlen(KEYBOARD_ROW3); ++i) {
      char c = KEYBOARD_ROW3[i];
      switch (letters_state[c - 'a']) {
        case LETTER_UNKNOWN:
          break;
        case LETTER_RIGHT:
          set_color_green();
          break;
        case LETTER_WRONG:
          set_color_yellow();
          break;
        case LETTER_ABSENT:
          set_color_grey();
          break;
      }
      putchar(c);
      reset_styles();
    }
    printf("\n\033[4A\033[2K");
    show_cursor();
#endif

    if (correct == WORDLEN) {
      quit(QUIT_CORRECT);
    }
  }

  printf("\033[2K\n\033[2K\n\033[2K\n\033[2K\033[2A");
  printf("The word was '%.*s'\n", (unsigned int) WORDLEN, word);
  printf("%.*s\n", (unsigned int) WORDDEFINITIONLEN, &buffer_definitions[word_index * WORDDEFINITIONLEN]);

  fflush(stdout);
  return 0;
}

