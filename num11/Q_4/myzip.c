
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char * argv[], char* envp[])
{
    int pipetar[2];
    if( pipe(pipetar) == -1) printf("%d", 1);

    pid_t value = fork();
    if (value == -1)
    {
        printf("Fork failed");
        exit(1);
    }
    
    if (!value)
    {
        if( dup2(pipetar[1],1) == -1) printf("%d", 4);
        if( close(pipetar[1]) == -1) printf("%d", 5);      
        
        char* tar_args[] = {"tar", "-c", argv[1], (char*) 0};
        execve("/bin/tar", tar_args, envp);
    
    }

    int pipezip[2];
    if( pipe(pipezip) == -1) printf("%d", 2);
    close(pipetar[1]);

    pid_t value2 = fork();
    if (value2 == -1)
    {
        printf("Fork failed");
        exit(1);
    }

    if (!value2)
    {

        if( dup2(pipezip[1],1) == -1) printf("%d", 7);
        if( close(pipezip[1]) == -1) printf("%d", 10);
        
        if( dup2(pipetar[0],0) == -1) printf("%d", 9);
        if( close(pipetar[0]) == -1) printf("%d", 11);
        

        char* gzip_args[] = {"gzip", (char*) 0};
        execve("/bin/gzip", gzip_args, envp);

    }

    if( close(pipetar[0]) == -1) fprintf(stderr, "%d", 11);
    if( close(pipezip[1]) == -1) fprintf(stderr, "%d", 11);

    pid_t value3 = fork();
    if (value3 == -1)
    {
        printf("Fork failed");
        exit(1);
    }

    if (!value3)
    {
        int fd = open("x.tar.gz.gpg", O_WRONLY | O_CREAT, 0644);
        dup2(fd,1);
        close(fd); 

        dup2(pipezip[0], 0);
        close(pipezip[0]);
        //char* gpg_args[] = {"gpg", "-c", (char*) 0};
        char* gpg_args[] = {"gpg", (char*) 0};
        execve("/bin/gpg", gpg_args, envp);
    }

    if( close(pipezip[0]) == -1) fprintf(stderr, "%d", 11);

    wait(&value3);
    wait(&value);
    wait(&value2);    
}