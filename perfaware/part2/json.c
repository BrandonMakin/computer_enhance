#include <stdio.h>
#include <stdlib.h>

#include "json.h"

void push_token(linked_list *list, char *source_data, token_type type, u32 start, u32 end)
{
    token *current_token = (token *)malloc(sizeof(token));
    current_token->type = type;
    current_token->string_pointer = source_data + start;
    current_token->length = end - start;

    list_push_back(list, current_token);
}

linked_list lex(char *data_string, u32 data_length)
{
    TimeFunction;
    linked_list tokens = list_create();
    u32 i = 0;

    while (i < data_length)
    {
        bool found_next_token = true;
        // lex single character symbols
        char c = data_string[i];
        if (
            c == '{' ||
            c == '}' ||
            c == '[' ||
            c == ']' ||
            c == ':' ||
            c == ','
        )
        {
            push_token(&tokens, data_string, (token_type)c, i, i+1);
            i++;
        }

        // lex whitespace
        else if (c == ' ' || c == '\n' || c == '\r')
        {
            i++;
        }

        // lex strings
        else if (c == '"')
        {
            u32 start = i + 1;
            while (i < data_length)
            {
                i++;
                c = data_string[i];
                if (c == '"')
                {
                    i++;
                    break;
                }
            }
            push_token(&tokens, data_string, TOKEN_STRING, start, i-1);
        }

        // lex numbers
        else
        {
            found_next_token = false;
            u32 start = i;
            while (
                i < data_length && (
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
                c = data_string[i];
                found_next_token = true;
                i++;
            }
            if (found_next_token)
            {
                i--;
                push_token(&tokens, data_string, TOKEN_NUMBER, start, i);
            }
        }
        // handle unrecognized characters
        if (found_next_token == false)
        {
            printf("unrecognized token: '%c' (at byte %d)\n", c, i);
            char* gray = "\033[90m";
            char* reset = "\033[0m";
            char* bold_white = "\033[1;97m";
            printf(gray); printf("%.*s", 20, &data_string[c-20]);
            printf(bold_white); printf("%c", c);
            printf(reset); printf(gray); printf("%.*s", 20, &data_string[c+1]);
            printf(reset);

            exit(1);
        }
    }

    return tokens;
}

json_object *parse(linked_list *tokens)
{
    TimeFunction;
    token *current_token = (token *)tokens->first->data;
    switch (current_token->type)
    {
        // parse arrays
        case '[':
        {
            if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
            current_token = (token *)tokens->first->data;

            json_object *return_value = (json_object *)calloc(1, sizeof(json_object));
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
                current_token = (token *)tokens->first->data;
                if (']' == current_token->type)
                {
                    if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
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
            current_token = (token *)tokens->first->data;

            hashtable json_dict = allocate_table();

            json_object *return_value = (json_object *)calloc(1, sizeof(json_object));
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
                    current_token = (token *)tokens->first->data;
                    switch (current_token->type)
                    {
                        case TOKEN_STRING:
                        {
                            char *key;
                            json_object *value;

                            {
                                u32 key_len = current_token->length;
                                key = (char *)calloc(key_len + 1, sizeof(char));
                                strncpy(key, current_token->string_pointer, key_len);
                            }

                            if (tokens->first != tokens->last) {tokens->first = tokens->first->next;}
                            current_token = (token *)tokens->first->data;

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
                            current_token = (token *)tokens->first->data;
                            
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
            bool is_negative = false;

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

            json_object *return_value = (json_object *)calloc(1, sizeof(json_object));
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
    TimeFunction;
    json_object *rv = parse(&tokens);
    return rv;
}

json_object *json_from_string(char *data_string, u32 data_length)
{
    // Lexical analysis
    linked_list tokens = lex(data_string, data_length);

    // Syntactic Analysis (Parsing)
    json_object *return_value = start_parse(tokens);

    return return_value;
}