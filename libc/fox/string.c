#include <stdint.h>
#include <string.h>

#include <fox/string.h>

static char malloc_buffer[131072];
static char *malloc_ptr = malloc_buffer;
static void *malloc(size_t size) {
    size = (size + 7UL) & ~((size_t) 7UL);
    void *ptr = malloc_ptr;
    malloc_ptr += size;
    return ptr;
}
static void free(const void *ptr) {
    (void) ptr;
    // TODO: send help
}

string_t string_from(const char *data) {
    string_t string = { data, strlen(data) };
    return string;
}

string_t string_copy(const char *data) {
    return string_clone(string_from(data));
}

string_t string_clone(string_t source) {
    size_t length = source.length, size = length + 1;

    char *data = malloc(size);
    memcpy(data, source.data, size);

    string_t string = { data, length };
    return string;
}

string_t string_slice(string_t source, size_t start, size_t end) {
    if (start > end || start >= source.length || end >= source.length) {
        return string_clone(S(""));
    }

    string_t string = { source.data + start, start - end };
    return string_clone(string);
}

string_t string_concat(string_t string1, string_t string2) {
    size_t length = string1.length + string2.length, size = length + 1;

    char *data = malloc(size);
    memcpy(data, string1.data, string1.length);
    memcpy(data + string1.length, string2.data, string2.length + 1);

    string_t string = { data, length };
    return string;
}

void string_free(string_t string) {
    free(string.data);
}

bool string_equals(string_t string1, string_t string2) {
    if (string1.length != string2.length) return false;
    return memcmp(string1.data, string2.data, string1.length) == 0;
}

string_t **string_tokenize(string_t source) {
    // list[0]: &S("echo")
    // list[1]: &S("hello")
    // list[2]: NULL
    string_t **list = malloc(sizeof(string_t *) * 64);
    string_t **list_head = list;
    for (size_t i = 0; i < 64; i++) list[i] = NULL;

    string_t current = S("");

    for (size_t i = 0; i < source.length; i++) {
        if (source.data[i] == ' ') {
            if (current.length > 0) {
                string_t *current_heap = malloc(sizeof(string_t));

                *current_heap = current;
                current = S("");

                *list_head = current_heap;
                list_head++;
            }
        } else {
            char character[2] = { source.data[i], 0 };
            current = string_concat(current, string_from(character));
        }
    }

    if (current.length > 0) {
        string_t *current_heap = malloc(sizeof(string_t));
        *current_heap = current;
        *list_head = current_heap;
    }

    return list;
}
