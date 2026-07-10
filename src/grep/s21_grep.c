#define _POSIX_C_SOURCE 200809L

#include "s21_grep.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../common/s21_common.h"

static void compile_pattern(const char* prog, GrepJob* job, const char* pat,
                            bool ignore_case) {
  if (job->pattern_count >= MAX_PATTERNS) {
    fprintf(stderr, "%s: too many patterns\n", prog);
    exit(S21_EXIT_USAGE);
  }
  int cflags = ignore_case ? REG_ICASE : 0;
  int rc = regcomp(&job->patterns[job->pattern_count], pat, cflags);
  if (rc != 0) {
    char errbuf[256];
    regerror(rc, &job->patterns[job->pattern_count], errbuf, sizeof(errbuf));
    fprintf(stderr, "%s: bad pattern '%s': %s\n", prog, pat, errbuf);
    exit(S21_EXIT_USAGE);
  }
  job->pattern_count++;
}

static void compile_patterns_from_file(const char* prog, GrepJob* job,
                                       const char* path, bool ignore_case,
                                       bool suppress_errors) {
  FILE* f = fopen(path, "r");
  if (!f) {
    if (!suppress_errors) {
      fprintf(stderr, "%s: %s: %s\n", prog, path, strerror(errno));
    }

    exit(S21_EXIT_USAGE);
  }
  char* line = NULL;
  size_t cap = 0;
  ssize_t len;
  while ((len = getline(&line, &cap, f)) != -1) {
    if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
    compile_pattern(prog, job, line, ignore_case);
  }
  free(line);
  fclose(f);
}

void grep_parse_args(int argc, char* argv[], GrepOptions* opts, GrepJob* job) {
  memset(opts, 0, sizeof(*opts));
  memset(job, 0, sizeof(*job));

  bool used_e = false;
  bool used_f = false;
  char* e_patterns[MAX_PATTERNS];
  int e_count = 0;
  char* f_files[MAX_PATTERNS];
  int f_count = 0;

  int flag;
  opterr = 0;
  while ((flag = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (flag) {
      case 'e':
        used_e = true;

        if (e_count >= MAX_PATTERNS) {
          fprintf(stderr, "%s: too many patterns\n", argv[0]);
          exit(S21_EXIT_USAGE);
        }

        e_patterns[e_count++] = optarg;
        break;
      case 'i':
        opts->ignore_case = true;
        break;
      case 'v':
        opts->invert_match = true;
        break;
      case 'c':
        opts->count_only = true;
        break;
      case 'l':
        opts->files_with_matches = true;
        break;
      case 'n':
        opts->line_number = true;
        break;
      case 'h':
        opts->no_filename = true;
        break;
      case 's':
        opts->suppress_errors = true;
        break;
      case 'f':
        used_f = true;

        if (f_count >= MAX_PATTERNS) {
          fprintf(stderr, "%s: too many pattern files\n", argv[0]);
          exit(S21_EXIT_USAGE);
        }

        f_files[f_count++] = optarg;
        break;
      case 'o':
        opts->only_matching = true;
        break;
      default:
        fprintf(stderr, "%s: invalid option\n", argv[0]);
        exit(S21_EXIT_USAGE);
    }
  }

  int remaining_start = optind;
  if (!used_e && !used_f) {
    if (remaining_start >= argc) {
      fprintf(stderr, "%s: missing pattern\n", argv[0]);
      exit(S21_EXIT_USAGE);
    }
    e_patterns[e_count++] = argv[remaining_start];
    remaining_start++;
  }

  for (int i = 0; i < e_count; i++) {
    compile_pattern(argv[0], job, e_patterns[i], opts->ignore_case);
  }
  for (int i = 0; i < f_count; i++) {
    compile_patterns_from_file(argv[0], job, f_files[i], opts->ignore_case,
                               opts->suppress_errors);
  }

  job->file_count = argc - remaining_start;
  if (job->file_count > 0) {
    job->files = &argv[remaining_start];
  }
}

static bool best_match(GrepJob* job, const char* text, bool notbol,
                       regmatch_t* out) {
  bool found = false;
  regmatch_t best = {-1, -1};
  for (int i = 0; i < job->pattern_count; i++) {
    regmatch_t m;
    int eflags = notbol ? REG_NOTBOL : 0;
    if (regexec(&job->patterns[i], text, 1, &m, eflags) == 0) {
      if (!found || m.rm_so < best.rm_so ||
          (m.rm_so == best.rm_so && m.rm_eo > best.rm_eo)) {
        best = m;
        found = true;
      }
    }
  }
  *out = best;
  return found;
}

static bool line_matches(GrepJob* job, const char* line) {
  regmatch_t m;
  return best_match(job, line, false, &m);
}

static void print_only_matching(GrepJob* job, const char* line,
                                const char* prefix, long lineno,
                                bool show_lineno) {
  const char* cursor = line;
  bool notbol = false;
  regmatch_t m;

  while (*cursor && best_match(job, cursor, notbol, &m)) {
    if (m.rm_so == m.rm_eo) {
      cursor++;
      notbol = true;
      continue;
    }

    if (prefix) printf("%s", prefix);

    if (show_lineno) printf("%ld:", lineno);

    printf("%.*s\n", (int)(m.rm_eo - m.rm_so), cursor + m.rm_so);

    cursor += m.rm_eo;
    notbol = true;
  }
}

static int process_stream(const char* prog, FILE* stream, const char* path,
                          const GrepOptions* opts, GrepJob* job,
                          const char* prefix) {
  (void)prog;
  char* line = NULL;
  size_t cap = 0;
  ssize_t len;
  long lineno = 0;
  long match_count = 0;
  bool any_match = false;

  while ((len = getline(&line, &cap, stream)) != -1) {
    lineno++;
    if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

    bool matched = line_matches(job, line);
    bool keep = opts->invert_match ? !matched : matched;
    if (!keep) continue;

    any_match = true;
    match_count++;

    if (opts->files_with_matches) break;
    if (opts->count_only) continue;
    if (opts->only_matching && opts->invert_match) continue;
    if (opts->only_matching) {
      print_only_matching(job, line, prefix, lineno, opts->line_number);
    } else {
      if (prefix) printf("%s", prefix);
      if (opts->line_number) printf("%ld:", lineno);
      printf("%s\n", line);
    }
  }
  free(line);

  if (opts->files_with_matches) {
    if (any_match) printf("%s\n", path ? path : "(standard input)");
  } else if (opts->count_only) {
    if (prefix) printf("%s", prefix);
    printf("%ld\n", match_count);
  }

  return any_match ? 0 : 1;
}

int grep_run(const char* prog, const GrepOptions* opts, GrepJob* job) {
  bool show_filename = job->file_count > 1 && !opts->no_filename;
  bool any_match = false;
  bool any_error = false;

  if (job->file_count == 0) {
    int rc = process_stream(prog, stdin, NULL, opts, job, NULL);
    return rc == 0 ? 0 : 1;
  }

  for (int i = 0; i < job->file_count; i++) {
    const char* path = job->files[i];
    FILE* f = fopen(path, "r");
    if (!f) {
      any_error = true;
      if (!opts->suppress_errors) {
        fprintf(stderr, "%s: %s: %s\n", prog, path, strerror(errno));
      }
      continue;
    }
    char prefix_buf[4096];
    const char* prefix = NULL;
    if (show_filename) {
      snprintf(prefix_buf, sizeof(prefix_buf), "%s:", path);
      prefix = prefix_buf;
    }
    int rc = process_stream(prog, f, path, opts, job, prefix);
    if (rc == 0) any_match = true;
    fclose(f);
  }

  if (any_match) return 0;
  if (any_error) return 2;
  return 1;
}

void grep_free_job(GrepJob* job) {
  for (int i = 0; i < job->pattern_count; i++) {
    regfree(&job->patterns[i]);
  }
}
