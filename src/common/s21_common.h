#ifndef S21_COMMON_H_
#define S21_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define S21_EXIT_SUCCESS 0
#define S21_EXIT_FAILURE 1
#define S21_EXIT_USAGE 2

void s21_perror(const char* prog, const char* path, const char* detail);

#endif  // S21_COMMON_H_
