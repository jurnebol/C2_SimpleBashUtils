#ifndef S21_GREP_H_
#define S21_GREP_H_

#include <regex.h>
#include <stdbool.h>

#define MAX_PATTERNS 256

typedef struct {
  bool ignore_case;
  bool invert_match;
  bool count_only;
  bool files_with_matches;
  bool line_number;
  bool no_filename;
  bool suppress_errors;
  bool only_matching;
} GrepOptions;

typedef struct {
  regex_t patterns[MAX_PATTERNS];
  int pattern_count;
  char** files;
  int file_count;
} GrepJob;

void grep_parse_args(int argc, char* argv[], GrepOptions* opts, GrepJob* job);
int grep_run(const char* prog, const GrepOptions* opts, GrepJob* job);
void grep_free_job(GrepJob* job);

#endif  // S21_GREP_H_
