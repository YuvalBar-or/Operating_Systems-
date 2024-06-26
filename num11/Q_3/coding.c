const int SIZEOFMSG = 55;

#include "coding.h"
#include "funcs.h"
#include <stdio.h>
#include <stdlib.h>

void code_or_decode(int option, char* src, char* dst)
{

    char key[62] = {'d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','1','2','3','4','5','6','7','8','9','0','a','b','c'};

    void* codec = createCodec(key);
    if (!codec)
    {
        printf("ilegal code\n");
        exit(1);
    }

    if (!option)
    {
        encode(src, dst, SIZEOFMSG ,codec);
    }
    else
    {
        decode(src, dst, SIZEOFMSG ,codec);
    }

    freecodec(codec);
    
    return;
    
}