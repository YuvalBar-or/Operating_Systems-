#include "coding.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("error - not enough arguments");
        exit(1);
    }
    
    char* src = argv[1];
    char* dst = argv[2];

    code_or_decode(1, src, dst);
    
    return 0;
}