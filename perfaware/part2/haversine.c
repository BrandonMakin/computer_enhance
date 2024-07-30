#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

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

static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */
    
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    

    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}

int main(int argc, char* argv[])
{
    char* file_name;
    if (argc > 1)
    {
        file_name = argv[1];
    }
    else
    {
        file_name = "haversine.json";
    }
    printf("reading: %s\n", file_name);
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

    f64 Average = 0;
    u64 Count = 0;

    json_object *data = json_from_string(file_data, file_length);
    json_object *pairs = table_get(data->dictionary, "pairs");

    list_node *current = pairs->list.first;
    for (int j = 0; /*j < 5 &&*/ current != NULL; j++)
    {
        Count++;
        current = current->next;
    }
    f64 SumCoefficient = 1/(f64)Count;
    // printf("SumCoef: %f\n", SumCoefficient);
    printf("pair count: %lli", Count);

    current = pairs->list.first;
    for (int j = 0; /*j < 5 &&*/ current != NULL; j++)
    {
        json_object *coordinates = current->data;
        f64 x0 = ((json_object* )table_get(coordinates->dictionary, "x0"))->number;
        f64 x1 = ((json_object* )table_get(coordinates->dictionary, "x1"))->number;
        f64 y0 = ((json_object* )table_get(coordinates->dictionary, "y0"))->number;
        f64 y1 = ((json_object* )table_get(coordinates->dictionary, "y1"))->number;

        // printf("\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f\t", x0, y0, x1, y1);

        f64 result = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
        // printf("haversine: %.16f\n", result);
        Average += result * SumCoefficient;
        // Count++;

        current = current->next;
    }

    // f64 Average = Sum / Count;
    printf("\nAverage: %.16f\n", Average);

    return 0;
}