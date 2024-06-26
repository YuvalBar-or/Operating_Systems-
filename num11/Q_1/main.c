#include <stdio.h>
#include "bugs.h"

int main()
{
    int bug_num = -1;
    printf("What bug type would you like to see? (1, 2, or 3) \n");
    scanf("%d", &bug_num);
    while ((bug_num != 1) && (bug_num != 2) && (bug_num != 3))
    {
        printf("%d is invalid bug_type \n", bug_num);
        printf("What bug type would you like to see? (1, 2, or 3) \n");
        scanf("%d", &bug_num);
    }
    
    if (bug_num == 1)
    {
        bugYuval();
    }
    else if (bug_num == 2)
    {
        bugOhad();
    }
    else //bug_num == 3
    {
        bugShira();
    }
    
    return 0;
}