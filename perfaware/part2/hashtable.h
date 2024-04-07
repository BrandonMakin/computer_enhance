#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef void* entry;
typedef entry* hashtable;

hashtable allocate_table(void);
void deallocate_table(hashtable *table);
entry table_get(hashtable table, char *key);
entry table_get_recursive(hashtable table, char **keys, char key_count);
bool table_set(hashtable table, char *key, entry value);
bool table_set_recursive(hashtable table, char **keys, char key_count, entry value);

// Here is a function to see this hashtable in action.
void hashtable_demo(void);

#endif //HASHTABLE_H