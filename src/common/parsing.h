#ifndef PARSING_H
#define PARSING_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define KEYS_SIZE 256
#define MAX_EXTRA_PATTERNS 128

// Структура для парсинга cat
typedef struct {
  char* key;
  char** files;
  int file_count;
} Parsing_struct_cat;

// Структура для парсинга grep
typedef struct {
  char** patterns;     // Массив шаблонов
  int patterns_count;  // Количество шаблонов
  char** files;        // Массив имен файлов
  int file_count;      // Количество файлов
  int key_e;           // Количество ключей -e
  int key_i;           // Флаг игнорирования регистра
  int key_v;           // Флаг инвертирования поиска
  int key_c;           // Флаг подсчета строк
  int key_l;  // Флаг вывода только имен файлов
  int key_n;  // Флаг вывода номеров строк
  int key_h;  // Флаг подавления имен файлов
  int key_s;  // Флаг подавления ошибок
  int key_f;  // Флаг чтения шаблонов из файла
  int key_o;  // Флаг вывода только совпадающих частей
  char incorrect_key;  // Некорректный ключ
  int incorrect_file;  // Некорректное имя файла
} Parsing_struct_grep;

// Функция для парсинга cat
Parsing_struct_cat main_parsing_cat(int argc, char* argv[]);

// Функция для парсинга grep
Parsing_struct_grep main_parsing_grep(int argc, char* argv[]);

// Функция для парсинга cat
void parsing_cat(int argc, char* argv[], Parsing_struct_cat* parsing);

// Функция для парсинга grep
void parsing_grep(int argc, char* argv[], Parsing_struct_grep* parsing);

FILE* open_file_safely_grep(char* file_name, int key_s);

#endif