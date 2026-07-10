#include "s21_common.h"

#include <errno.h>

void s21_perror(const char* prog, const char* path, const char* detail) {
  if (detail) {
    fprintf(stderr, "%s: %s: %s\n", prog, path, detail);
  } else {
    fprintf(stderr, "%s: %s: %s\n", prog, path, strerror(errno));
  }
}
