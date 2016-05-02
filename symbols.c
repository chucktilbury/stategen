
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "symbols.h"
#include "errors.h"

void destroy_symbol(symbol_t *ptr) {

    symbol_t *sym = ptr;
    if(NULL != sym->name)
        free(sym->name);
    if(UNION_TYPE_STRING == sym->union_type)
        if(NULL != sym->data.string)
            free(sym->data.string);
    free(sym);
}

symbol_t *create_symbol(char *name, int type, int subtype, int union_type, void *data) {

    symbol_t *sym;

    if(NULL == (sym = (symbol_t *)calloc(1, sizeof(symbol_t))))
        SERROR(FATAL_ERROR, "Cannot allocate symbol struct");

    if(NULL == (sym->name = strdup(name)))
        SERROR(FATAL_ERROR, "Cannot allocate symbol name");

    sym->type = type;
    sym->subtype = subtype;
    sym->union_type = union_type;
    switch(union_type) {
        case UNION_TYPE_STRING:
            if(NULL == (sym->data.string = strdup(data)))
                SERROR(FATAL_ERROR, "Cannot allocate symbol data");
            break;

        case UNION_TYPE_NUMBER:
            sym->data.number = *((unsigned int*)data);
            break;

        case UNION_TYPE_GENERIC:
            sym->data.generic = data;
            break;

        case UNION_TYPE_VOID:
            sym->data.number = 0;
            break;

        default:
            SERROR(FATAL_ERROR, "Unknown union type");
    }
    return sym;
}

symbol_table_h symbol_table_create(int size) {
    hash_callback hcb = (hash_callback)destroy_symbol;
    return (symbol_table_h)hash_table_create(size, hcb);
}

void symbol_table_destroy(symbol_table_h handle) {
    hash_table_destroy(handle);
}

symbol_t *symbol_table_find(symbol_table_h handle, char *name) {
    return (symbol_t *)hash_table_find(handle, name);
}

int symbol_table_add(symbol_table_h handle, char *name, symbol_t *sym) {
    return hash_table_add(handle, name, (void*)sym);
}

int symbol_table_delete(symbol_table_h handle, char *name) {
    return hash_table_delete(handle, name);
}

int symbol_table_set_value(symbol_table_h handle, char *name, symbol_t *sym) {
    return hash_table_set_value(handle, name, (void*)sym);
}
