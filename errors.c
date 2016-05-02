
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>

#include "files.h"
#include "errors.h"

static int num_errors = 0;
//static int num_warnings = 0;

void *allocate_mem(char *file, int line, size_t size) {

    void *ptr;

    if(NULL == (ptr = calloc(1, size))) {
        fprintf(stderr, "FATAL_ERROR: Cannot allocate %d bytes of memory\n", size);
        exit(1);
    }
    return ptr;
}

char *string_dup(char *file, int line, char *str) {

    char *strg;
    if(NULL == (strg = strdup(str))) {
        fprintf(stderr, "FATAL_ERROR: Cannot allocate %d bytes for a string\n", strlen(str));
        exit(1);
    }
    return strg;
}

void show_error(int type, char *file, int line, char *fmt, ...) {

    va_list args;
    char *fstrg;

    va_start(args, fmt);
    num_errors++;

    switch(type) {
        case FATAL_ERROR:
            fprintf(stderr, "FATAL ERROR: %s: %d: ", file, line);
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, "\n");
            exit(1);
            break;

        case EPARSE_ERROR:
            fprintf(stderr, "SYNTAX ERROR: %s: ", file);
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, "\n");
            exit(1);
            break;

        case FILE_ERROR:
            fstrg = strerror(errno);
            fprintf(stderr, "FILE ERROR: %s: %d: ", file, line);
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, ": %s\n", fstrg);
            exit(1);
            break;

        case SCAN_ERROR:
            fprintf(stderr, "SCAN ERROR: %s: %d: ", file_name(), line_number());
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, "\n");
            break;

        case PARSE_ERROR:
            fprintf(stderr, "PARSE ERROR: %s: %d: ", file_name(), line_number());
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, "\n");
            break;

        case EMIT_ERROR:
            fstrg = strerror(errno);
            fprintf(stderr, "EMIT ERROR: ");
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, ": %s\n", fstrg);
            exit(1);
            break;

        case SYNTAX_ERROR:
            fprintf(stderr, "SYNTAX ERROR: %s: %d: ", file_name(), line_number());
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, "\n");
            break;

        default:
            fprintf(stderr, "UNKNOWN ERROR: %s: ", file);
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, "\n");
            exit(1);
            break;
    }

}

//int get_warnings(void) { return num_warnings; }
int get_errors(void) { return num_errors; }
