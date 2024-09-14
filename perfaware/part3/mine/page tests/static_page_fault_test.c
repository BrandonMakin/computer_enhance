#include <stdint.h>

volatile uint8_t data[1024*1024*1024];

int main()
{
    printf("Press enter to start\n");  
    getchar();

    for (int i = 0; i < 1024*1024*1024; ++i)
    {
        data[i] = 1;
    }

    printf("Done. Press enter to exit\n");  
    getchar();
}