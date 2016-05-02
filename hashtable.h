
#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef void *hash_table_h;
typedef void (*hash_callback)(void*);

hash_table_h hash_table_create(int size, hash_callback hc);
void hash_table_destroy(hash_table_h handle);
void *hash_table_find(hash_table_h handle, char *name);
int hash_table_add(hash_table_h handle, char *name, void *value);
int hash_table_delete(hash_table_h handle, char *name);
int hash_table_set_value(hash_table_h handle, char *name, void *value);

#endif /* HASHTABLE_H */
