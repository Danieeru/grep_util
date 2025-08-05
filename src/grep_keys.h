#ifndef GREP_KEYS_H
#define GREP_KEYS_H
#include <ctype.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

regex_t *patterns_to_regexes(char **patterns, int patterns_count);
regex_t *patterns_to_regexes_with_i(char **patterns, int patterns_count);
FILE *create_temp_file_with_lower_patterns(FILE *file);

void main_print(FILE *file, char *file_name, regex_t *regexes,
                int patterns_count, int file_count, int key_n, int key_v,
                int key_h, int key_o);

void print_with_c(FILE *file, char *file_name, regex_t *regexes,
                  int patterns_count, int file_count, int key_v, int key_h);
void print_with_l(FILE *file, char *file_name, regex_t *regexes,
                  int patterns_count, int key_v);
void print_with_f(FILE *file, char *file_name, regex_t *regexes,
                  int patterns_count, int file_count);

#endif
