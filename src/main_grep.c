#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "common/parsing.h"
#include "grep_keys.h"

bool check_incorrect_key(Parsing_struct_grep parsing);
bool check_exist_pattern(Parsing_struct_grep parsing, int key_s);
bool read_file(Parsing_struct_grep parsing, char *file_name);

int main(int argc, char *argv[]) {
  Parsing_struct_grep parsing = main_parsing_grep(argc, argv);
  if (parsing.incorrect_file) {
    return 1;
  }
  if (!check_incorrect_key(parsing)) {
    return 1;
  }
  if (!check_exist_pattern(parsing, parsing.key_s)) {
    return 1;
  }
  if (parsing.file_count == 0) {
    if (!read_file(parsing, NULL)) {
      return 1;
    }
  } else {
    for (int i = 0; i < parsing.file_count; i++) {
      read_file(parsing, parsing.files[i]);
    }
  }
  for (int i = 0; i < parsing.patterns_count; i++) free(parsing.patterns[i]);
  free(parsing.patterns);
  for (int i = 0; i < parsing.file_count; i++) free(parsing.files[i]);
  free(parsing.files);
  return 0;
}

bool check_incorrect_key(Parsing_struct_grep parsing) {
  if (parsing.incorrect_key != '\0') {
    if (parsing.incorrect_key == 'e' || parsing.incorrect_key == 'f') {
      printf("grep: option requires an argument -- '%c'\n",
             parsing.incorrect_key);
      printf("Usage: grep [OPTION]... PATTERNS [FILE]...\n");
      printf("Try 'grep --help' for more information.\n");
    } else {
      printf("grep: invalid option -- '%c'\n", parsing.incorrect_key);
      printf("Usage: grep [OPTION]... PATTERNS [FILE]...\n");
      printf("Try 'grep --help' for more information.\n");
    }
    return false;
  }
  return true;
}

bool read_file(Parsing_struct_grep parsing, char *file_name) {
  FILE *file;
  if (file_name == NULL) {
    file = stdin;
  } else {
    file = open_file_safely_grep(file_name, parsing.key_s);
  }
  if (file == NULL) {
    return false;
  }
  regex_t *regexes;
  if (parsing.key_i) {
    regexes =
        patterns_to_regexes_with_i(parsing.patterns, parsing.patterns_count);
  } else {
    regexes = patterns_to_regexes(parsing.patterns, parsing.patterns_count);
  }
  if (regexes == NULL) {
    return false;
  }
  if (parsing.key_l) {
    print_with_l(file, file_name, regexes, parsing.patterns_count,
                 parsing.key_v);
  } else if (parsing.key_c) {
    print_with_c(file, file_name, regexes, parsing.patterns_count,
                 parsing.file_count, parsing.key_v, parsing.key_h);
  } else {
    main_print(file, file_name, regexes, parsing.patterns_count,
               parsing.file_count, parsing.key_n, parsing.key_v, parsing.key_h,
               parsing.key_o);
  }
  for (int i = 0; i < parsing.patterns_count; i++) {
    regfree(&regexes[i]);
  }
  free(regexes);
  fclose(file);
  return true;
}

bool check_exist_pattern(Parsing_struct_grep parsing, int key_s) {
  // Отсутствие шаблона может быть только используя флаг -e
  if (parsing.patterns_count == 0) {
    if (key_s == 0) {
      printf("grep: option requires an argument -- 'e'\n");
      printf("Usage: grep [OPTION]... PATTERNS [FILE]...\n");
      printf("Try 'grep --help' for more information.\n");
    }
    return false;
  }
  return true;
}