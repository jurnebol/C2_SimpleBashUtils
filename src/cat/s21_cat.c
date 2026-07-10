#include "s21_cat.h"

#include <ctype.h>
#include <getopt.h>
#include <string.h>

#include "../common/s21_common.h"

static const struct option kLongOptions[] = {
    {"number-nonblank", no_argument, NULL, 'b'},
    {"number", no_argument, NULL, 'n'},
    {"squeeze-blank", no_argument, NULL, 's'},
    {"show-ends", no_argument, NULL, 'E'},
    {"show-tabs", no_argument, NULL, 'T'},
    {"show-nonprinting", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0},
};

int cat_parse_args(int argc, char* argv[], CatOptions* opts) {
  memset(opts, 0, sizeof(*opts));
  int flag;
  opterr = 0;

  while ((flag = getopt_long(argc, argv, "benstvET", kLongOptions, NULL)) !=
         -1) {
    switch (flag) {
      case 'b':
        opts->number_nonblank = true;
        break;
      case 'e':
        opts->show_ends = true;
        opts->show_nonprinting = true;
        break;
      case 'E':
        opts->show_ends = true;
        break;
      case 'n':
        opts->number_all = true;
        break;
      case 's':
        opts->squeeze_blank = true;
        break;
      case 't':
        opts->show_tabs = true;
        opts->show_nonprinting = true;
        break;
      case 'T':
        opts->show_tabs = true;
        break;
      case 'v':
        opts->show_nonprinting = true;
        break;
      default:
        fprintf(stderr, "s21_cat: invalid option\n");
        exit(S21_EXIT_USAGE);
    }
  }
  return optind;
}

static void put_nonprinting(unsigned char c, const CatOptions* opts) {
  if (c == '\t' && !opts->show_tabs) {
    putchar(c);
    return;
  }
  if (c == '\n') {
    putchar(c);
    return;
  }
  if (c >= 128) {
    printf("M-");
    c -= 128;
  }
  if (c == 127) {
    printf("^?");
  } else if (c < 32) {
    if (c == '\t' && opts->show_tabs) {
      printf("^I");
    } else {
      printf("^%c", c + '@');
    }
  } else {
    putchar(c);
  }
}

int cat_file(const char* prog, const char* path, const CatOptions* opts) {
  FILE* stream = strcmp(path, "-") == 0 ? stdin : fopen(path, "rb");
  if (!stream) {
    s21_perror(prog, path, NULL);
    return 1;
  }

  static long line_number = 1;
  static bool at_line_start = true;
  static int consecutive_blank_lines = 0;

  int c;
  while ((c = getc(stream)) != EOF) {
    if (at_line_start) {
      bool line_is_blank = (c == '\n');
      if (opts->squeeze_blank && line_is_blank) {
        consecutive_blank_lines++;
        if (consecutive_blank_lines > 1) {
          at_line_start = true;
          continue;
        }
      } else {
        consecutive_blank_lines = 0;
      }

      bool should_number =
          opts->number_all || (opts->number_nonblank && !line_is_blank);
      if (opts->number_nonblank) should_number = !line_is_blank;
      if (should_number) {
        const char* sep =
            (opts->show_nonprinting || opts->show_tabs) ? "  " : "\t";
        printf("%6ld%s", line_number, sep);
      }
      if (opts->number_all || opts->number_nonblank) {
        if (should_number) line_number++;
      }
      at_line_start = false;
    }

    if (c == '\n') {
      if (opts->show_ends) putchar('$');
      putchar('\n');
      at_line_start = true;
      continue;
    }

    if (opts->show_nonprinting || opts->show_tabs) {
      put_nonprinting((unsigned char)c, opts);
    } else {
      putchar(c);
    }
  }

  if (stream != stdin) fclose(stream);
  return 0;
}
