#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "grep_keys.h"

#include "common/parsing.h"

// Функция для экранирования спецсимволов в паттерне
char *escape_special_pattern(const char *pattern) {
  const char *specials = "*+?{}|()[]\\";
  if (pattern && strlen(pattern) == 1 && strchr(specials, pattern[0])) {
    char *escaped = malloc(3);
    if (!escaped) {
      return NULL;
    }
    escaped[0] = '\\';
    escaped[1] = pattern[0];
    escaped[2] = '\0';
    return escaped;
  }
  return strdup(pattern);
}

regex_t *patterns_to_regexes(char **patterns, int patterns_count) {
  int reti;
  regex_t *regexes = malloc(patterns_count * sizeof(regex_t));
  for (int i = 0; i < patterns_count; i++) {
    char *escaped_pattern = escape_special_pattern(patterns[i]);
    if (!escaped_pattern) {
      return NULL;
    }
    reti = regcomp(&regexes[i], escaped_pattern, REG_EXTENDED);
    free(escaped_pattern);
    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      return NULL;
    }
  }
  return regexes;
}
regex_t *patterns_to_regexes_with_i(char **patterns, int patterns_count) {
  int reti;
  regex_t *regexes = malloc(patterns_count * sizeof(regex_t));
  for (int i = 0; i < patterns_count; i++) {
    char *escaped_pattern = escape_special_pattern(patterns[i]);
    if (!escaped_pattern) {
      return NULL;
    }
    reti = regcomp(&regexes[i], escaped_pattern, REG_EXTENDED | REG_ICASE);
    free(escaped_pattern);
    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      return NULL;
    }
  }
  return regexes;
}

FILE *create_temp_file_with_lower_patterns(FILE *file) {
  FILE *temp_file = tmpfile();
  if (temp_file == NULL) {
    fprintf(stderr, "Could not create temp file\n");
    return NULL;
  }
  char *line_buf = NULL;
  size_t line_buf_size = 0;
  while (getline(&line_buf, &line_buf_size, file) != -1) {
    char *current = line_buf;
    while (*current != '\0') {
      if (isupper(*current)) {
        *current = tolower(*current);
      }
      current++;
    }
    fwrite(line_buf, 1, strlen(line_buf), temp_file);
    free(line_buf);
    line_buf = NULL;
  }
  // Перематываем временный файл в начало для последующего чтения
  rewind(temp_file);
  return temp_file;
}

void find_best_match(char *current, regex_t *regexes, int patterns_count,
                     regmatch_t *best_match, int *found_pattern) {
  int min_so = -1;
  int min_eo = -1;
  for (int i = 0; i < patterns_count; i++) {
    regmatch_t match;
    if (regexec(&regexes[i], current, 1, &match, 0) == 0) {
      int match_len = match.rm_eo - match.rm_so;
      int best_len = (min_so != -1) ? (min_eo - min_so) : -1;
      if (min_so == -1 || match.rm_so < min_so ||
          (match.rm_so == min_so && match_len > best_len)) {
        min_so = match.rm_so;
        min_eo = match.rm_eo;
        *found_pattern = i;
      }
    }
  }
  best_match->rm_so = min_so;
  best_match->rm_eo = min_eo;
}

void print_filename_or_linenumber(bool should_print_filename, char *file_name,
                                  int line_number, int key_n) {
  if (should_print_filename) {
    if (key_n == 1) {
      printf("%s%s%d%s", file_name, ":", line_number, ":");
    } else if (key_n == 0) {
      printf("%s%s", file_name, ":");
    }
  } else {
    if (key_n == 1) {
      printf("%d%s", line_number, ":");
    }
  }
}

void print_line(char *line_buf) { printf("%s", line_buf); }

void main_print(FILE *file, char *file_name, regex_t *regexes,
                int patterns_count, int file_count, int key_n, int key_v,
                int key_h, int key_o) {
  char *line_buf = NULL;
  size_t line_buf_size = 0;
  ssize_t nread;
  int line_number = 1;
  bool should_print_filename =
      file_count > 1 && file_name != NULL && key_h == 0;
  while ((nread = getline(&line_buf, &line_buf_size, file)) != -1) {
    bool found = false;
    int regexec_result;
    for (int i = 0; i < patterns_count; i++) {
      regexec_result = regexec(&regexes[i], line_buf, 0, NULL, 0);
      if (regexec_result == 0) {
        found = true;
        break;
      }
    }
    if (key_v == 0 && found == true) {
      if (key_o == 0)
        print_filename_or_linenumber(should_print_filename, file_name,
                                     line_number, key_n);
      char *current = line_buf;
      regmatch_t best_match;
      while (regexec_result == 0) {
        int found_pattern = -1;
        find_best_match(current, regexes, patterns_count, &best_match,
                        &found_pattern);
        // Печать строки после до и после найденных шаблонов и выход из цикла
        if (found_pattern == -1 || best_match.rm_eo == best_match.rm_so) {
          if (key_o == 0) {
            print_line(current);
          }
          break;
        }
        if (key_o == 0) {
          printf("%.*s", best_match.rm_so,
                 current);  // Печать строки до шаблона
          printf("%.*s", best_match.rm_eo - best_match.rm_so,
                 current + best_match.rm_so);  // Печать шаблона
        } else {
          print_filename_or_linenumber(should_print_filename, file_name,
                                       line_number, key_n);
          printf("%.*s\n", best_match.rm_eo - best_match.rm_so,
                 current + best_match.rm_so);
        }
        current += best_match.rm_eo;
        if (*current == '\0') {
          break;
        }
      }
    } else if (key_v == 1 && found == false && key_o == 0) {
      print_filename_or_linenumber(should_print_filename, file_name,
                                   line_number, key_n);
      print_line(line_buf);
    }
    line_number++;
  }
  if (line_buf[strlen(line_buf)] != '\0' && key_v == 0) {
    printf("\n");
  }
  free(line_buf);
}

void print_with_c(FILE *file, char *file_name, regex_t *regexes,
                  int patterns_count, int file_count, int key_v, int key_h) {
  char *line_buf = NULL;
  size_t line_buf_size = 0;
  ssize_t nread;
  int count = 0;
  while ((nread = getline(&line_buf, &line_buf_size, file)) != -1) {
    bool found = false;
    int regexec_result;
    for (int i = 0; i < patterns_count; i++) {
      regexec_result = regexec(&regexes[i], line_buf, 0, NULL, 0);
      if (regexec_result == 0) {
        found = true;
        break;
      }
    }
    if ((key_v >= 1 && found == false) || (key_v == 0 && found == true))
      count++;
  }
  if (file_count > 1 && file_name != NULL && key_h == 0) {
    printf("%s%s%d\n", file_name, ":", count);
  } else {
    printf("%d\n", count);
  }
  free(line_buf);
}

void print_with_l(FILE *file, char *file_name, regex_t *regexes,
                  int patterns_count, int key_v) {
  ssize_t nread;
  char *line_buf = NULL;
  size_t line_buf_size = 0;
  bool has_matching_line = false;  // есть ли строка, подходящая под условие

  while ((nread = getline(&line_buf, &line_buf_size, file)) != -1) {
    bool found = false;
    int regexec_result;
    for (int i = 0; i < patterns_count; i++) {
      regexec_result = regexec(&regexes[i], line_buf, 0, NULL, 0);
      if (regexec_result == 0) {
        found = true;
        break;
      }
    }

    // Для -v: строка подходит, если НЕ найдена ни одним шаблоном
    // Для обычного поиска: строка подходит, если найдена хотя бы одним шаблоном
    if ((key_v == 1 && !found) || (key_v == 0 && found)) {
      has_matching_line = true;
      break;  // достаточно найти одну такую строку
    }
  }

  // Печатаем имя файла, если есть хотя бы одна подходящая строка
  if (has_matching_line) {
    printf("%s\n", file_name);
  }

  free(line_buf);
}