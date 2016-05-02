
#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "hashtable.h"

enum {
    UNION_TYPE_STRING,
    UNION_TYPE_NUMBER,
    UNION_TYPE_GENERIC,
    UNION_TYPE_VOID,
};

typedef struct {
    char *name;
    int type;
    int subtype;
    int union_type;
    union {
        unsigned int number;
        char *string;
        void *generic;
    } data;
} symbol_t;

typedef hash_table_h symbol_table_h;

symbol_t *create_symbol(char *name, int type, int subtype, int union_type, void *data);
void destroy_symbol(symbol_t *ptr);
symbol_table_h symbol_table_create(int size);
void symbol_table_destroy(symbol_table_h handle);
symbol_t *symbol_table_find(symbol_table_h handle, char *name);
int symbol_table_add(symbol_table_h handle, char *name, symbol_t *sym);
int symbol_table_delete(symbol_table_h handle, char *name);
int symbol_table_set_value(symbol_table_h handle, char *name, symbol_t *sym);

#endif /* SYMBOLS_H */
