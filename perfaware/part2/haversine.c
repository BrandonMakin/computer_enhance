#include <stdio.h>

#include "hashtable.h"
#include "linked_list.h"

int main(int argc, char* argv[])
{
    puts("\033[32mLinked List Demo:\033[0m");
    linked_list_demo();

    puts("\n\033[32mHashtable Demo:\033[0m");
    hashtable_demo();
}