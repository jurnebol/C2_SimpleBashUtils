#ifndef S21_CAT_H_
#define S21_CAT_H_

#include <stdbool.h>
#include <stdio.h>

typedef struct {
  bool number_nonblank;
  bool show_ends;
  bool show_nonprinting;
  bool number_all;
  bool squeeze_blank;
  bool show_tabs;
} CatOptions;

int cat_parse_args(int argc, char* argv[], CatOptions* opts);

int cat_file(const char* prog, const char* path, const CatOptions* opts);

#endif  // S21_CAT_H_
