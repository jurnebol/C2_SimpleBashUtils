#include "s21_grep.h"

int main(int argc, char* argv[]) {
  GrepOptions opts;
  GrepJob job;

  grep_parse_args(argc, argv, &opts, &job);
  int exit_code = grep_run(argv[0], &opts, &job);
  grep_free_job(&job);

  return exit_code;
}
