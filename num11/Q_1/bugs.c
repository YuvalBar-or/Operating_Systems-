#include <stdlib.h>

int bugYuval()
{
    int x = 5 / 0;
    x++;
    return 0;
}

int bugOhad()
{
    int* arr = malloc(5);
    arr[809305734] = 95;
    free(arr);
    return 0;
}

int bugShira()
{
    bugShira();
    return 0;
}