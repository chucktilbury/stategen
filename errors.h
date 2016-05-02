
#ifndef ERRORS_H
#define ERRORS_H

#define SERROR(t, fmt, ... ) show_error(t, __FILE__, __LINE__, fmt, ## __VA_ARGS__)
#define PERROR(n, fmt, ... ) show_error(EPARSE_ERROR, n, 0, fmt, ## __VA_ARGS__)
#define ALLOC(t)    allocate_mem(__FILE__, __LINE__, sizeof(#t))
#define STRDUP(s)   string_dup(__FILE__, __LINE__, s)

enum {
    FATAL_ERROR,
    SCAN_ERROR,
    PARSE_ERROR,
    EPARSE_ERROR,
    FILE_ERROR,
    EMIT_ERROR,
    SYNTAX_ERROR,
};

void show_error(int type, char *file, int line, char *fmt, ...);
int get_errors(void);
//int get_warnings(void);
void *allocate_mem(char *file, int line, size_t size);
char *string_dup(char *file, int line, char *str);

#endif /* ERRORS_H */
