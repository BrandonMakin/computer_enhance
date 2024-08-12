#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;

typedef void* entry;
typedef entry* hashtable;

// My simple, janky little hash so that I can have a hashtable.
// It gives a unique u8 for all the dictionary keys found in my input data.
u8 hash(char* string)
{
    size_t length = strlen(string);
    u8 result = 0;
    for (size_t i = 0; i < length; i++)
    {
        char letter = string[i];
        result = (result ^ letter) * 2;
    }
    return result % 11;
}

hashtable allocate_table(void)
{
    hashtable table = (hashtable)calloc(11, sizeof(entry));
    return table;
}

void deallocate_table(hashtable *table)
{
    free(*table);
    *table = NULL;
}

entry table_get(hashtable table, char *key)
{
    return table[hash(key)];
}

bool table_set(hashtable table, char *key, entry value)
{
    if (value == NULL) {return false;}
    if (table_get(table, key) != NULL) {return false;}

    table[hash(key)] = value;
    return true;
}

entry table_get_recursive(hashtable table, char **keys, char key_count)
{
    if (key_count == 0) {return NULL;}

    hashtable current = table;
    for (int i = 0; i < key_count - 1; ++i)
    {
        current = (hashtable)table_get(current, keys[i]);
    }

    return table_get(current, keys[key_count - 1]);
}

bool table_set_recursive(hashtable table, char **keys, char key_count, entry value)
{
    if (key_count == 0) {return false;}

    hashtable current = table;
    for (int i = 0; i < key_count - 1; ++i)
    {
        hashtable new_table = (hashtable)table_get(current, keys[i]);
        if (new_table == NULL)
        {
            new_table = allocate_table();
            table_set(current, keys[i], new_table);
        }
        current = new_table;
    }
    return table_set(current, keys[key_count - 1], value);
}

void hashtable_demo(void)
{
    // create the data
    hashtable table = allocate_table();
    char key1[] = "montgomery";
    char msg1[] = "ricky";
    char msg2[] = "you're a pond and i'm an ocean.";

    table_set(table, key1, msg1);
    
    hashtable subtable = allocate_table();
    table_set(subtable, "loverman", "i'm headed straight for the floor.");
    table_set(table, "ricky", subtable);
    table_set(subtable, "line without a hook", msg2);

    hashtable subsubtable = allocate_table();
    table_set(subtable, "california", subsubtable);
    table_set(subsubtable, "why", "oh baby?");

    char *keys[] = {"ricky", "california", "this evening", "I'll", "be", "headed"};
    table_set_recursive(table, keys, 5, "back");

    // use the data
    printf("Printing table_get with \"%s\"\n", key1);
    puts((char *)table_get(table, key1));

    puts("");
    puts("Printing table_get with \"loverman\"");
    puts((char *)table_get(subtable, "loverman"));

    puts("");
    puts("Printing \"loverman\" by getting a table within a table");
    hashtable subtable_ref_2 = (hashtable)table_get(table, "ricky");
    puts((char *)table_get(subtable_ref_2, "loverman"));

    puts("");
    puts("Printing \"line without a hook\" as a subtable one-liner");
    puts((char *)table_get((hashtable)table_get(table, "ricky"), "line without a hook"));

    puts("");
    puts("Printing \"loverman\" with table_get_recursive");
    char* strs1[] = {"ricky", "loverman"};
    puts((char *)table_get_recursive(table, strs1, 2));
    
    puts("");
    puts("table_get_recursive stress test");
    char* strs2[] = {"ricky", "california", "why"};
    puts((char *)table_get_recursive(table, strs2, 3));
    puts((char *)table_get_recursive(table, keys, 5));
}