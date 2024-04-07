#include <stdio.h>
#include <stdint.h>

#include "hashtable.h"
#include "linked_list.h"

typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;

typedef struct token
{
    u32 start_index;
    u32 end_index;
} token;

void push_token(linked_list *list, u32 start, u32 end)
{
    token *current_token = (token* )malloc(sizeof(token));
    current_token->start_index = start;
    current_token->end_index = end;

    list_node *node = list_push_back(list);
    node->data = current_token;
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
    char* converted_length = (char* )calloc(30, sizeof(char));
    if (file_length > 1<<30)
        sprintf(converted_length, " (%d GiB)", file_length/(1<<30));
    else if (file_length > 1<<20)
        sprintf(converted_length, " (%d MiB)", file_length/(1<<20));
    else if (file_length > 1<<10)
        sprintf(converted_length, " (%d KiB)", file_length/(1<<10));
    printf("(File is %d bytes%s long.)\n", file_length, converted_length);

    fseek(file, 0L, SEEK_SET);

    char* file_data = (char* )calloc(file_length, sizeof(char));
    // using calloc instead of malloc to initialize bytes to 0 may be pointless
    // because I immediately overwrite the data by reading from a file
    // but maybe it'll prevent some weird bug I otherwise never would've found.
    
    // fread(file_data, sizeof(char), 1000, file);
    fread(file_data, sizeof(char), file_length, file);
    fclose(file);
    file = NULL;

    // printf("%s\n", file_data);

    // Lexical analysis
    
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
            push_token(&tokens, i, i+1);
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
            push_token(&tokens, start, i-1);
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
                push_token(&tokens, start, i);
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

            return 1;
        }
    }
    puts("Lexical analysis completed successfully!");



    // // Debug: print lexer tokens
    // list_node *current_node = tokens.first;
    // puts("\033[31mTokens:\033[0m");
    // i = 0;
    // while (current_node != NULL && i < 50)
    // {
    //     token *current_token = current_node->data;
    //     i32 string_length = current_token->end_index - current_token->start_index;
    //     char* blue = "\033[36m";
    //     char* reset = "\033[0m";
    //     if (string_length > 0 && string_length < 20)
    //     {
    //         printf("%s`%s", blue, reset);
    //         printf("%.*s", string_length, &file_data[current_token->start_index]);
    //         printf("%s`%s ", blue, reset);
    //     }

    //     current_node = current_node->next;
    //     i++;
    // }
    // puts("");
    
    // token *current_token = tokens.last->data;
    // i32 string_length = current_token->end_index - current_token->start_index;
    // printf("%.*s", string_length, &file_data[current_token->start_index]);

    return 0;
}