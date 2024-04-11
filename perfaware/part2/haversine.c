#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "hashtable.h"
#include "linked_list.h"
#include "json.h"

typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;

void pretty_print_data_size(data_size)
{
    printf("(File is %d bytes", data_size);
    if (data_size > (1<<30))
        printf(" (%d GiB)", data_size/(1<<30));
    else if (data_size > 1<<20)
        printf(" (%d MiB)", data_size/(1<<20));
    else if (data_size > 1<<10)
        printf(" (%d KiB)", data_size/(1<<10));
    printf(" long.)\n");
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
    fseek(file, 0L, SEEK_SET);

    pretty_print_data_size(file_length);


    char* file_data = calloc(file_length, sizeof(char));
    // using calloc instead of malloc to initialize bytes to 0 may be pointless
    // because I immediately overwrite the data by reading from a file
    // but maybe it'll prevent some weird bug I otherwise never would've found.
    
    fread(file_data, sizeof(char), file_length, file);
    fclose(file);
    file = NULL;

    json_object *coordinates = json_from_string(file_data, file_length);

    puts(coordinates->type == JSON_DICTIONARY ? "true" : "false");

    json_object *pairs = table_get(coordinates->dictionary, "pairs");
    puts(pairs->type == JSON_LIST ? "true" : "false");

    list_node *current = pairs->list.first;
    for (int j = 0; j < 5; j++)
    {
        json_object *coord = current->data;
        puts(coord->type == JSON_DICTIONARY ? "idx is a dict" : "idx is not a dict");

        printf("address of x0: %p\n", table_get(coord->dictionary, "x0"));
        printf("address of x1: %p\n", table_get(coord->dictionary, "x1"));
        printf("address of y0: %p\n", table_get(coord->dictionary, "y0"));
        printf("address of y1: %p\n", table_get(coord->dictionary, "y1"));
    }
    puts("");

    return 0;
}