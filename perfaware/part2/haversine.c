#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "hashtable.h"
#include "linked_list.h"

typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
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

void push_token(linked_list *list, char *source_data, token_type type, u32 start, u32 end)
{
    token *current_token = malloc(sizeof(token));
    current_token->type = type;
    current_token->string_pointer = source_data + start;
    current_token->length = end - start;

    list_push_back(list, current_token);
}

linked_list lex(char *file_data, u32 file_length)
{
    printf("(%d): %.200s\n", file_length, file_data);
    linked_list tokens = list_create();
    u32 i = 0;

    while (i < file_length)
    {
        bool found_next_token = true;
        // lex single character symbols
        char c = file_data[i];
        if (
            c == '{' ||
            c == '}' ||
            c == '[' ||
            c == ']' ||
            c == ':' ||
            c == ','
        )
        {
            push_token(&tokens, file_data, c, i, i+1);
            i++;
        }

        // lex whitespace
        else if (c == ' ')
        {
            i++;
        }

        // lex strings
        else if (c == '"')
        {
            u32 start = i + 1;
            while (i < file_length)
            {
                i++;
                c = file_data[i];
                if (c == '"')
                {
                    i++;
                    break;
                }
            }
            push_token(&tokens, file_data, TOKEN_STRING, start, i-1);
        }

        // lex numbers
        else
        {
            found_next_token = false;
            u32 start = i;
            while (
                i < file_length && (
                c == '-' ||
                c == '0' ||
                c == '1' ||
                c == '2' ||
                c == '2' ||
                c == '3' ||
                c == '4' ||
                c == '5' ||
                c == '6' ||
                c == '7' ||
                c == '8' ||
                c == '9' ||
                c == '.' ||
                c == 'e' ||
                c == 'E' 
            ))
            {
                c = file_data[i];
                found_next_token = true;
                i++;
            }
            if (found_next_token)
            {
                i--;
                push_token(&tokens, file_data, TOKEN_NUMBER, start, i);
            }
        }
        // handle unrecognized characters
        if (found_next_token == false)
        {
            printf("unrecognized token: '%c' (at byte %d)\n", c, i);
            char* gray = "\033[90m";
            char* reset = "\033[0m";
            char* bold_white = "\033[1;97m";
            printf(gray); printf("%.*s", 20, &file_data[c-20]);
            printf(bold_white); printf("%c", c);
            printf(reset); printf(gray); printf("%.*s", 20, &file_data[c+1]);
            printf(reset);

            exit(1);
        }
    }

    return tokens;
}

json_object *parse(linked_list *tokens)
{
    token *current_token = tokens->first->data;
    switch (current_token->type)
    {
        // parse arrays
        case '[':
        {
            if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
            current_token = tokens->first->data;

            json_object *return_value = calloc(1, sizeof(json_object));
            return_value->type = JSON_LIST;
            return_value->list = list_create();
            if (']' == current_token->type)
            {
                if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                return return_value;
            }
            while (tokens->first != NULL)
            {
                json_object *result = parse(tokens);
                // printf("parsing: %p\t", result);
                list_push_back(&return_value->list, result);
                // printf("json_list.last: %p\t", json_list.last);
                // puts((result == json_list.last) ? "matches" : "doesn't match");
                current_token = tokens->first->data;
                if (']' == current_token->type)
                {
                    if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                    printf("return_value->list.first: %p\n", return_value->list.first);
                    return return_value;
                }
                else if (',' == current_token->type)
                {
                    if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                }
                else
                {
                    printf("Expected comma after array object. Got token of type: %c", current_token->type);
                    printf(" (%.*s)\n", current_token->length, current_token->string_pointer);
                    exit(1);
                }
            }
            printf("Expected ']' at the end of an array. Got EOL instead.\n");
            exit(1);
            return NULL;
        }

        // parse objects
        case '{':
        {
            if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
            current_token = tokens->first->data;

            hashtable json_dict = allocate_table();

            json_object *return_value = calloc(1, sizeof(json_object));
            return_value->type = JSON_DICTIONARY;
            return_value->dictionary = json_dict;

            if ('}' == current_token->type)
            {
                if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                return return_value;
            }
            else
            {
                while (tokens->first != NULL)
                {
                    current_token = tokens->first->data;
                    switch (current_token->type)
                    {
                        case TOKEN_STRING:
                        {
                            char *key;
                            json_object *value;

                            {
                                u32 key_len = current_token->length;
                                key = calloc(key_len + 1, sizeof(char));
                                strncpy(key, current_token->string_pointer, key_len);
                            }

                            if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                            current_token = tokens->first->data;

                            if (':' != current_token->type)
                            {
                                printf("Expected ':' after object key. Got token of type: %c\n", current_token->type);
                                
                                exit(1);
                                return NULL;
                            }

                            if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                            value = parse(tokens);

                            table_set(json_dict, key, value);

                            if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                            current_token = tokens->first->data;
                            
                            if (',' != current_token->type)
                            {
                                if ('}' != current_token->type)
                                {
                                    printf("Expected ',' or '}' after object value. Got token of type: %c\n", current_token->type);
                                    
                                    exit(1);
                                    return NULL;
                                }
                                else
                                {
                                    if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                                    return return_value;
                                }
                            }

                            if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}

                            break;
                        }
                        default:
                        {
                            printf("Expected '}' or a string. Got token of type: %c\n", current_token->type);
                            
                            exit(1);
                            return NULL;
                        }
                    }
                }
                printf("Expected '}' at the end of an array. Got EOL instead.\n");
            }

            exit(1);
            return NULL;
        }

        // parse numbers
        case TOKEN_NUMBER:
        {
            // TODO (maybe) replace all this with a call to `sscanf()` or `strtod()`
            char *string = current_token->string_pointer;
            u32 string_length = current_token->length;
            
            f64 number = 0;
            i64 exponent = 0;

            u32 i = 0;
            bool is_negative;

            if ('-' == string[0])
            {                
                if (1 == string_length)
                {
                    puts("invalid number: this number is just a negative sign (-).");

                    exit(1);
                    return NULL;
                }
                else
                {
                    is_negative = true;
                    ++i;
                }
            }
            
            if ('.' == string[0] ||
                'e' == string[0] ||
                'E' == string[0] )
            {
                printf("invalid number: this number starts with something other than a digit (%c).\n", string[0]);

                exit(1);
                return NULL;
            }

            bool after_decimal_place = false;
            bool after_exponent = false;
            f64 j = 10;
            for ( ; i < string_length; ++i)
            {
                if ('e' == string[i] ||
                    'E' == string[i])
                {
                    after_exponent = true;
                    continue;
                }
                else if ('.' == string[i])
                {
                    after_decimal_place = true;
                    continue;
                }

                if (after_exponent)
                {
                    exponent *= 10;
                    exponent += string[i] - '0';
                }
                else
                { 
                    if (after_decimal_place)
                    {
                        number += (string[i] - '0')/j;
                        j *= 10;
                    }
                    else // digits before the decimal place.
                    {
                        number *= 10;
                        number += string[i] - '0';
                    }
                }
            }
            if (exponent == 0) {exponent = 1;}
            number *= exponent;
            if (is_negative) {number *= -1;}

            json_object *return_value = calloc(1, sizeof(json_object));
            return_value->type = JSON_NUMBER;
            return_value->number = number;
            return return_value;
        }

        // parse strings
        case TOKEN_STRING:
        {
            puts("String parsing is not implemented yet.");

            exit(1);
            return NULL;
        }

        default:
        {
            printf("oh no, we got something I didn't expect of type: %c\n", current_token->type);
            
            exit(1);
            return NULL;
        }
    }
}

json_object *start_parse(linked_list tokens)
{
    json_object *rv = parse(&tokens);
    return rv;
}

int main(int argc, char* argv[])
{
    char* file_name = "haversine.json";
    FILE *file = fopen(file_name, "rb");
    if (file == NULL)
    {
        printf("Can't open \"%s\"\n", file_name);
        return 1;
    }

    // Get the length of the file by reading in binary mode, seeking to the end
    // and saving the file position indicator.
    fseek(file, 0, SEEK_END);
    u32 file_length = ftell(file);

    // DEBUG: print file length.
    char* converted_length = calloc(30, sizeof(char));
    if (file_length > 1<<30)
        sprintf(converted_length, " (%d GiB)", file_length/(1<<30));
    else if (file_length > 1<<20)
        sprintf(converted_length, " (%d MiB)", file_length/(1<<20));
    else if (file_length > 1<<10)
        sprintf(converted_length, " (%d KiB)", file_length/(1<<10));
    printf("(File is %d bytes%s long.)\n", file_length, converted_length);

    fseek(file, 0L, SEEK_SET);

    char* file_data = calloc(file_length, sizeof(char));
    // using calloc instead of malloc to initialize bytes to 0 may be pointless
    // because I immediately overwrite the data by reading from a file
    // but maybe it'll prevent some weird bug I otherwise never would've found.
    
    fread(file_data, sizeof(char), file_length, file);
    fclose(file);
    file = NULL;

    // Lexical analysis
    linked_list tokens = lex(file_data, file_length);
    puts("Lexical analysis completed successfully!");

    // Syntactic Analysis (Parsing)
    json_object *coordinates = start_parse(tokens);
    puts("Syntactic analysis completed successfully!");

    
    
    return 0;
}