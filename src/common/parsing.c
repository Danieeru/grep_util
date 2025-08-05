#define _GNU_SOURCE
#include "parsing.h"

#include <string.h>
#include <unistd.h>

Parsing_struct_cat main_parsing_cat(int argc, char* argv[]) {
  Parsing_struct_cat parsing;
  parsing_cat(argc, argv, &parsing);
  return parsing;
}

void parsing_cat(int argc, char* argv[], Parsing_struct_cat* parsing) {
  bool check_one_key = true;
  parsing->file_count = 0;
  parsing->files = NULL;
  parsing->key = NULL;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == '-') {
        if (check_one_key) {
          parsing->key = malloc(strlen(argv[i]) + 1);  // +1 для '\0'
          if (parsing->key != NULL) {
            strcpy(parsing->key, argv[i]);  // Сохраняем ключ вместе с "--"
            check_one_key = false;
          }
        }
      } else {
        // Для короткого ключа (-k)
        if (check_one_key) {
          parsing->key = malloc(strlen(argv[i]) + 1);  // +1 для '\0'
          if (parsing->key != NULL) {
            strcpy(parsing->key, argv[i]);  // Сохраняем ключ вместе с "-"
            check_one_key = false;
          }
        }
      }
    } else {
      // Выделение памяти для имен файлов
      char** temp =
          realloc(parsing->files, (parsing->file_count + 1) * sizeof(char*));
      if (temp != NULL) {
        parsing->files = temp;
        parsing->files[parsing->file_count] = argv[i];
        (parsing->file_count)++;
      }
    }
  }
}
/*========================================================================================*/

Parsing_struct_grep main_parsing_grep(int argc, char* argv[]) {
  Parsing_struct_grep parsing;
  parsing_grep(argc, argv, &parsing);
  return parsing;
}
void init_parsing_grep(Parsing_struct_grep* parsing) {
  parsing->patterns = NULL;
  parsing->patterns_count = 0;
  parsing->file_count = 0;
  parsing->files = NULL;
  parsing->key_e = 0;
  parsing->key_i = 0;
  parsing->key_v = 0;
  parsing->key_c = 0;
  parsing->key_l = 0;
  parsing->key_n = 0;
  parsing->key_h = 0;
  parsing->key_s = 0;
  parsing->key_f = 0;
  parsing->key_o = 0;
  parsing->incorrect_key = '\0';
  parsing->incorrect_file = 0;
}

FILE* open_file_safely_grep(char* file_name, int key_s) {
  FILE* file = fopen(file_name, "r");
  if (file == NULL) {
    if (key_s == 0) {
      fprintf(stderr, "grep: %s: No such file or directory\n", file_name);
    }
    return NULL;
  }
  return file;
}

// Функция для проверки уникальности шаблона
static bool is_pattern_unique(char* pattern,
                              const Parsing_struct_grep* parsing) {
  for (int i = 0; i < parsing->patterns_count; i++) {
    if (parsing->patterns[i] != NULL &&
        strcmp(pattern, parsing->patterns[i]) == 0) {
      return false;
    }
  }
  return true;
}

// Функция для добавления шаблона
static void add_pattern(Parsing_struct_grep* parsing, char* pattern) {
  if (pattern == NULL || *pattern == '\0') {
    return;
  }
  char* pattern_copy = strdup(pattern);
  if (!pattern_copy) {
    return;
  }
  char* saveptr = NULL;
  char* token = strtok_r(pattern_copy, "\n", &saveptr);
  while (token != NULL) {
    if (*token != '\0' && is_pattern_unique(token, parsing)) {
      char** temp = realloc(parsing->patterns,
                            sizeof(char*) * (parsing->patterns_count + 1));
      if (temp != NULL) {
        parsing->patterns = temp;
        parsing->patterns[parsing->patterns_count] = malloc(strlen(token) + 1);
        if (parsing->patterns[parsing->patterns_count] != NULL) {
          strcpy(parsing->patterns[parsing->patterns_count], token);
          parsing->patterns_count++;
        }
      }
    }
    token = strtok_r(NULL, "\n", &saveptr);
  }
  free(pattern_copy);
}

static void add_patterns_from_file(Parsing_struct_grep* parsing,
                                   char* file_name) {
  if (parsing->incorrect_file) {
    return;
  }
  FILE* file = open_file_safely_grep(file_name, 0);
  if (file == NULL) {
    parsing->incorrect_file = 1;
    return;
  }
  char* line = NULL;
  size_t line_len = 0;
  while (getline(&line, &line_len, file) != -1) {
    line[strcspn(line, "\n")] = '\0';
    add_pattern(parsing, line);
  }
  free(line);
  fclose(file);
}

static bool is_valid_key(char key) {
  return (key == 'e' || key == 'i' || key == 'v' || key == 'c' || key == 'l' ||
          key == 'n' || key == 'h' || key == 's' || key == 'f' || key == 'o');
}

static bool find_e_key(int argc, char* argv[], Parsing_struct_grep* parsing) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (int j = 1; argv[i][j] != '\0'; j++) {
        if (argv[i][j] == 'e') {
          // Проверяем, что все предыдущие символы в этом аргументе - валидные
          // ключи
          for (int k = 1; k < j; k++) {
            if (!is_valid_key(argv[i][k])) {
              parsing->incorrect_key = argv[i][k];
              return false;
            }
          }
          return true;
        }
      }
    }
  }
  return false;
}

static bool find_f_key(int argc, char* argv[], Parsing_struct_grep* parsing) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (int j = 1; argv[i][j] != '\0'; j++) {
        if (argv[i][j] == 'f') {
          for (int k = 1; k < j; k++) {
            if (!is_valid_key(argv[i][k])) {
              parsing->incorrect_key = argv[i][k];
              return false;
            }
          }
          return true;
        }
      }
    }
  }
  return false;
}

static void parse_with_e_or_f(int argc, char* argv[],
                              Parsing_struct_grep* parsing) {
  bool expect_pattern = false;
  bool expect_file_with_patterns = false;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && expect_pattern == false &&
        expect_file_with_patterns == false) {
      // Обработка ключей
      for (int j = 1; argv[i][j] != '\0'; j++) {
        switch (argv[i][j]) {
          case 'e':
            parsing->key_e++;
            expect_pattern = true;
            // Если после -e сразу идет шаблон (например -ea)
            if (argv[i][j + 1] != '\0') {
              add_pattern(parsing, &argv[i][j + 1]);
              expect_pattern = false;
              j = strlen(argv[i]) - 1;
            }
            break;
          case 'f':
            parsing->key_f++;
            expect_file_with_patterns = true;
            // Если после -f сразу идет файл с шаблонами (например
            // -fpattern.txt)
            if (argv[i][j + 1] != '\0') {
              add_patterns_from_file(parsing, &argv[i][j + 1]);
              expect_file_with_patterns = false;
              j = strlen(argv[i]) - 1;
            }
            break;
          case 'i':
            parsing->key_i = 1;
            break;
          case 'v':
            parsing->key_v = 1;
            break;
          case 'c':
            parsing->key_c = 1;
            break;
          case 'l':
            parsing->key_l = 1;
            break;
          case 'n':
            parsing->key_n = 1;
            break;
          case 'h':
            parsing->key_h = 1;
            break;
          case 's':
            parsing->key_s = 1;
            break;
          case 'o':
            parsing->key_o = 1;
            break;
          default:
            if (parsing->incorrect_key == '\0') {
              parsing->incorrect_key = argv[i][j];
            }
            break;
        }
      }
    } else if (expect_pattern) {
      // Обработка шаблона после -e
      add_pattern(parsing, argv[i]);
      expect_pattern = false;
    } else if (expect_file_with_patterns) {
      // Обработка файла с шаблонами после -f
      add_patterns_from_file(parsing, argv[i]);
      expect_file_with_patterns = false;
      expect_pattern = false;
    } else {
      // Все остальные нефлаги = файлы
      char** temp =
          realloc(parsing->files, sizeof(char*) * (parsing->file_count + 1));
      if (temp != NULL) {
        parsing->files = temp;
        parsing->files[parsing->file_count] = malloc(strlen(argv[i]) + 1);
        if (parsing->files[parsing->file_count] != NULL) {
          strcpy(parsing->files[parsing->file_count], argv[i]);
          parsing->file_count++;
        }
      }
    }
  }
  // Проверяем, не остался ли ожидающий шаблон
  if (expect_pattern && parsing->key_e >= 1 && parsing->incorrect_key == '\0') {
    parsing->incorrect_key = 'e';
  } else if (expect_file_with_patterns && parsing->key_f == 1 &&
             parsing->incorrect_key == '\0') {
    parsing->incorrect_key = 'f';
  }
}

static void parse_without_e_and_f(int argc, char* argv[],
                                  Parsing_struct_grep* parsing) {
  bool pattern_found = false;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      // Обработка ключей
      for (int j = 1; argv[i][j] != '\0'; j++) {
        switch (argv[i][j]) {
          case 'i':
            parsing->key_i = 1;
            break;
          case 'v':
            parsing->key_v = 1;
            break;
          case 'c':
            parsing->key_c = 1;
            break;
          case 'l':
            parsing->key_l = 1;
            break;
          case 'n':
            parsing->key_n = 1;
            break;
          case 'h':
            parsing->key_h = 1;
            break;
          case 's':
            parsing->key_s = 1;
            break;
          case 'f':
            parsing->key_f = 1;
            break;
          case 'o':
            parsing->key_o = 1;
            break;
          default:
            if (parsing->incorrect_key == '\0') {
              parsing->incorrect_key = argv[i][j];
            }
            break;
        }
      }
    } else if (!pattern_found) {
      // Первый нефлаг = основной шаблон
      add_pattern(parsing, argv[i]);
      pattern_found = true;
    } else {
      // Все остальные нефлаги = файлы
      char** temp =
          realloc(parsing->files, sizeof(char*) * (parsing->file_count + 1));
      if (temp != NULL) {
        parsing->files = temp;
        parsing->files[parsing->file_count] = malloc(strlen(argv[i]) + 1);
        if (parsing->files[parsing->file_count] != NULL) {
          strcpy(parsing->files[parsing->file_count], argv[i]);
          parsing->file_count++;
        }
      }
    }
  }
}

void parsing_grep(int argc, char* argv[], Parsing_struct_grep* parsing) {
  init_parsing_grep(parsing);
  bool has_e = find_e_key(argc, argv, parsing);
  bool has_f = find_f_key(argc, argv, parsing);
  if (has_e || has_f) {
    parse_with_e_or_f(argc, argv, parsing);
  } else {
    parse_without_e_and_f(argc, argv, parsing);
  }
}