#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include <math.h>

#define ID_WIDTH 10
#define NAME_WIDTH 40
#define STATUS_WIDTH 15
#define ASSIGNEES_WIDTH 80

struct MemoryStruct {
  char *memory;
  size_t size;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
void print_table_separator();
void print_table_header();
void print_table_row(const char *id, const char *name, const char *status, const char *assignees);

#endif /* MAIN_H */
