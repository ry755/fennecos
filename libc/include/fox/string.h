#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct string_s {
    const char *data;
    size_t length;
} string_t;

#define S(_literal) ((string_t) { (_literal), sizeof(_literal) - 1 })

string_t string_from(const char *data);
string_t string_copy(const char *data);
string_t string_clone(string_t source);
string_t string_slice(string_t source, size_t start, size_t end);
void string_free(string_t string);
bool string_equals(string_t string1, string_t string2);
string_t **string_tokenize(string_t source);
