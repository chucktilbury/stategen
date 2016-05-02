
/*
 *  Hash table implementation.
 *
 *  Nothing outside of this file needs access to the hash table data strucutre.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hashtable.h"
#include "errors.h"

typedef struct __hte__ {
    char *name;
    void *value;
    int status;
    struct __hte__ *next;
} hash_table_entry_t;

typedef struct {
    hash_table_entry_t **table;
    int size;
    void (*free_func)(void *val);
} hash_table_t;

hash_table_h hash_table_create(int size, hash_callback hcb) {

    hash_table_t *ht;

    if(NULL == (ht = (hash_table_t *)calloc(1, sizeof(hash_table_t))))
        SERROR(FATAL_ERROR, "Cannot allocate hash table struct");

    if(NULL == (ht->table = (hash_table_entry_t **)calloc(size, sizeof(hash_table_entry_t))))
        SERROR(FATAL_ERROR, "Cannot allocate hash table");

    ht->free_func = hcb;
    ht->size = size;

    return (hash_table_h)ht;

}

void hash_table_destroy(hash_table_h handle) {

    hash_table_t *table = (hash_table_t *)handle;
    hash_table_entry_t *next, *hte;

    if(NULL != table) {
        int i;
        for(i = 0; i < table->size; i++) {
            if(NULL != table->table[i]) {
                for(hte = table->table[i]; hte != NULL; hte = next) {
                    next = hte->next;
                    if(NULL != hte->name) {
                        free(hte->name);
                    }

                    if(NULL != hte->value) {
                        if(NULL != table->free_func) {
                            (*table->free_func)(hte->value);
                        }
                    }
                    free(hte);
                }
            }
        }
    }
}

static inline int make_hash(char *str, int size) {

    unsigned int hash = 5381;
    int i;

    for(i = 0; 0 != str[i]; i++)
        hash = ((hash << 5) + hash) + str[i];
    return hash % size;
}

/*
 *  Local find function.
 */
static inline hash_table_entry_t *find_local(hash_table_t *table, char *name, int hash) {

    hash_table_entry_t *hte;

    for(hte = table->table[hash]; NULL != hte; hte = hte->next) {
        if(!strcmp(name, hte->name)) {
            return hte; // found
        }
    }
    return NULL;    // not found
}


void *hash_table_find(hash_table_h handle, char *name) {

    hash_table_entry_t *hte;
    hash_table_t *table = (hash_table_t *)handle;
    int hash;

    if(NULL != table) {
        hash = make_hash(name, table->size);
        hte = find_local(table, name, hash);
        if(NULL != hte)
            if(hte->status == 0)
                return hte->value;  // success
            else
                return NULL;    // was deleted
        else
            return NULL;    // not found
    }
    else
        return NULL;    // invalid table
}

int hash_table_add(hash_table_h handle, char *name, void *value) {

    int hash;
    hash_table_entry_t *hte;
    hash_table_t *table = (hash_table_t *)handle;

    if(NULL != table) {
        hash = make_hash(name, table->size);
        hte = find_local(table, name, hash);
        if(NULL == hte) {
            if(NULL == (hte = (hash_table_entry_t *)calloc(1, sizeof(hash_table_entry_t))))
                SERROR(FATAL_ERROR, "Cannot allocate table entry");

            if(NULL == (hte->name = strdup(name)))
                SERROR(FATAL_ERROR, "Cannot allocate table entry name");

            hte->value = value;
            hte->status = 0;
            hte->next = NULL;

            if(NULL != table->table[hash])
                hte->next = table->table[hash];
            table->table[hash] = hte;

            return 0;   // success
        }
        else if(0 != hte->status) {
            hte->status = 0;
            return hash_table_set_value(table, name, value);
        }
        else {
            return 1;   // symbol already in the table
        }
    }
    else {
        return -1;      // table is invalid
    }
}

int hash_table_delete(hash_table_h handle, char *name) {

    hash_table_t *table = (hash_table_t *)handle;
    hash_table_entry_t *hte;
    int hash;

    if(NULL != table) {
        hash = make_hash(name, table->size);
        hte = find_local(table, name, hash);
        if(NULL != hte) {
            hte->status = 1;
            return 0;   // success
        }
        else {
            return 1;   // not found
        }
    }
    else {
        return -1;  // invalid table
    }
}

int hash_table_set_value(hash_table_h handle, char *name, void *value) {

    hash_table_t *table = (hash_table_t *)handle;
    hash_table_entry_t *hte;
    int hash;

    if(NULL != table) {
        hash = make_hash(name, table->size);
        hte = find_local(table, name, hash);
        if(NULL != hte) {
            if(NULL != hte->value) {
                if(NULL != table->free_func) {
                    (*table->free_func)(hte->value);
                }
            }
            hte->value = value;
            return 0;   // success
        }
        else {
            return 1;   // not found
        }
    }
    else {
        return -1;  // invalid table
    }
}



#ifdef UNIT_TEST

/*
 *  The value of the symbol.  This can be any arbritrary data structure.
 *  It is normally handled by the caller.  All the symbol table does is
 *  store the data under the name.
 */
typedef struct {
    int type;
    int subtype;
} symbol_value_t;

struct ss {
    char *name;
    symbol_value_t val;
} static_symbols[] = {
    {"blue",    {24, 23}},
    {"green",   {25, 99}},
    {"yellow",  {26, 45}},
    {"pink",    {27, 34}},
    {"orange",  {28, 56}},
    {"purple",  {29, 78}},
    {"white",   {30, 47}},
    {"copper",  {31, 67}},
    {"gray",    {32, 98}},
    {"brown",   {33, 72}},
    {NULL,      {-1, -1}}
};

void *create_value(int type, int subtype) {
    symbol_value_t *val;

    if(NULL == (val = (symbol_value_t*)calloc(1, sizeof(symbol_value_t)))) {
        fprintf(stderr, "TEST ERROR: cannot allocate value\n");
        exit(1);
    }

    val->subtype = subtype;
    val->type = type;

    return val;
}

void free_value(void *val) {
    free(val);
}

int main(void) {

    hash_table_t *tab = hash_table_create(2048, free_value);
    int i;

    for(i = 0; static_symbols[i].name != NULL; i++) {
        void *val = create_value(static_symbols[i].val.type, static_symbols[i].val.subtype);
        if(0 != hash_table_add(tab, static_symbols[i].name, val)) {
            fprintf(stderr, "TEST ERROR: cannot add symbol: \"%s\"\n", static_symbols[i].name);
            exit(1);
        }
    }

    for(i = 0; static_symbols[i].name != NULL; i++) {
        symbol_value_t *val = (symbol_value_t *)hash_table_find(tab, static_symbols[i].name);
        if(val == NULL) {
            fprintf(stderr, "TEST ERROR: symbol \"%s\" not found\n", static_symbols[i].name);
            exit(1);
        }
        else {
            fprintf(stderr, "symbol: \"%s\": %d %d\n", static_symbols[i].name, val->type, val->subtype);
        }
    }

    hash_table_delete(tab, "green");
    hash_table_delete(tab, "copper");

    for(i = 0; static_symbols[i].name != NULL; i++) {
        symbol_value_t *val = (symbol_value_t *)hash_table_find(tab, static_symbols[i].name);
        if(val == NULL) {
            fprintf(stderr, "symbol \"%s\" not found\n", static_symbols[i].name);
        }
        else {
            fprintf(stderr, "symbol: \"%s\": %d %d\n", static_symbols[i].name, val->type, val->subtype);
        }
    }

    // load an entire dictionary.
    FILE *fp;
    if(NULL == (fp = fopen("dictionary.txt", "r"))) {
        fprintf(stderr, "TEST ERROR: cannot open dictionary: ");
        perror("");
        exit(1);
    }

    char buffer[1024];
    int lines = 1;

    for(lines = 1; fgets(buffer, sizeof(buffer), fp); lines++) {
        char *sp = strchr(buffer, '\n');
        if(sp != NULL)
            *sp = 0;

        void *val = create_value(1000, lines);
        if(0 != hash_table_add(tab, buffer, val)) {
            fprintf(stderr, "symbol: \"%s\" already exists\n", buffer);
           //exit(1);
        }
    }
    fclose(fp);
    fprintf(stderr, "added %d symbols\n", lines);

    for(i = 0; static_symbols[i].name != NULL; i++) {
        symbol_value_t *val = (symbol_value_t *)hash_table_find(tab, static_symbols[i].name);
        if(val == NULL) {
            fprintf(stderr, "symbol \"%s\" not found\n", static_symbols[i].name);
        }
        else {
            fprintf(stderr, "symbol: \"%s\": %d %d\n", static_symbols[i].name, val->type, val->subtype);
        }
    }

    fprintf(stderr, "\n");
    hash_table_entry_t *ent;
    for(ent = tab->table[0]; ent != NULL; ent = ent->next)
        fprintf(stderr, "row 0: %s\n", ent->name);


    hash_table_destroy(tab);
    return 0;
}

#endif
