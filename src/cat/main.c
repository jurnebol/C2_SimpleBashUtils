#include <stdio.h>

#include "s21_cat.h"

int main(int argc, char* argv[]) {
  CatOptions opts;
  int first_file = cat_parse_args(argc, argv, &opts);

  int exit_code = 0;
  if (first_file >= argc) {
    exit_code |= cat_file(argv[0], "-", &opts);
  } else {
    for (int i = first_file; i < argc; i++) {
      exit_code |= cat_file(argv[0], argv[i], &opts);
    }
  }

  return exit_code;
}
