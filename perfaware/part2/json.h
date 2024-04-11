#ifndef JSON_H
#define JSON_H

#include <stdint.h>

#include "hashtable.h"
#include "linked_list.h"

typedef uint32_t u32;
typedef int64_t i64;

typedef double f64;

typedef enum token_type
{
    TOKEN_COMMA = ',',
    TOKEN_COLON = ':',
    TOKEN_LEFT_BRACKET = '[',
    TOKEN_RIGHT_BRACKET = ']',
    TOKEN_LEFT_BRACE = '{',
    TOKEN_RIGHT_BRACE = '}',
    // STRING and NUMBER have ascii chars so that I can easily print them with "%c"
    // But those two might as well be equal to 0 and 1, or anything else.
    TOKEN_NUMBER = 'N',
    TOKEN_STRING = 'S',
} token_type;

typedef enum json_object_type
{
    JSON_STRING,
    JSON_LIST,
    JSON_NUMBER,
    JSON_DICTIONARY,
} json_object_type;

typedef struct token
{
    token_type type;
    char *string_pointer;
    u32 length;
} token;

typedef struct json_object
{
    json_object_type type;
    union
    {
        char* string;
        linked_list list;
        f64 number;
        hashtable dictionary;
    };
} json_object;

json_object *json_from_string(char *data_string, u32 data_length);

#endif