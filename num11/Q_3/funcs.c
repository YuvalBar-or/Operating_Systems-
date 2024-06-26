#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "funcs.h"

void* createCodec (char key[62])
{
    int tmp[62] = {0};
    char* codec = (char*) malloc(124);
    if (!codec)
    {
        printf("Error - malloc failed\n");
        exit(1);
    }
    
    int VALID = 1;
    for (int i = 0; i < 62; i++)
    {
        char cur = key[i];
        int place = place_in_62(cur);
        if (place == -1)
        {
            VALID = 0;
        }
        else
        {
            if (tmp[place]==0)
            {
                tmp[place]++;
                codec[i] = cur;
                codec[(place+62)] = index_to_char(i);
            }
            else VALID = 0; 
        }
        if (!VALID)
        {
            free(codec);
            return NULL;
        }
    }
    return codec;
}

int place_in_62(char ch)
{
    if (ch >= 'a' && ch <= 'z') return (ch-'a');
    else if (ch >= 'A' && ch <= 'Z') return (ch - 'A' + 26);
    else if (ch >= '0' && ch <= '9') return (ch - '0' + 52);
    else return -1;
}

char index_to_char(int idx)
{
    if (idx >= 0 && idx < 26)
    {
        return ((char) ('a' + idx));
    }
    else if (idx >= 26 && idx < 52)
    {
        return ((char) ('A' + idx - 26));
    }
    else if (idx >= 52 && idx < 62)
    {
        return ((char) ('0' + idx - 52));
    }
    else
    {
        return '!';
    }
}

int encode(char * textin, char * textout, int len, void * codec)
{
    char* codec1 = (char*) codec;
    if (!textin || !textout || !codec)
    {
        printf("Error - nullptr\n");
        exit(1);
    }

    FILE* read = fopen(textin, "r");
    FILE* write = fopen(textout, "w");
    if (read == NULL || write == NULL)
    {
        printf("Error - file not found\n");
        exit(1);
    }

    int chars_coded = 0;
    for (int i = 0; i < len; i++)
    {
        char cur = ((char) fgetc(read));
        int place = place_in_62(cur);
        if (place == -1)
        {
            fputc(cur, write);
        }
        else
        {
            fputc(codec1[place], write);
            chars_coded++;
        }
    }
    fputc('\n', write);
    fclose(read);
    fclose(write);
    return chars_coded;
}

int decode(char * textin, char * textout, int len, void * codec)
{
    char* codec1 = (char*) codec;
    if (!textin || !textout || !codec)
    {
        printf("Error - nullptr\n");
        exit(1);
    }

    FILE* read = fopen(textin, "r");
    FILE* write = fopen(textout, "w");
    if (read == NULL || write == NULL)
    {
        printf("Error - file not found\n");
        exit(1);
    }

    int chars_encoded = 0;
    for (int i = 0; i < len; i++)
    {
        char cur = ((char) fgetc(read));
        int place = place_in_62(cur);
        if (place == -1)
        {
            fprintf(write, "%c", cur);
        }
        else
        {
            place = place + 62;
            fputc(codec1[place], write);
            chars_encoded++;
        }
    }
    fputc('\n', write);
    fclose(read);
    fclose(write);
    return chars_encoded;
}

void freecodec(void * codec)
{
    free(codec);
    return;
}