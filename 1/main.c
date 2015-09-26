#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    /*
    if (argc < 2)
    {
        printf("Arguments number too small.\n");
        return 1;
    }
    execvp(argv[1], argv + 1);
    printf("Exec error\n");
    return 0;
    */

    if (argc != 2)
    {
        printf("Invalid arguments.\n");
        return 1;
    }

    char *endptr = NULL;
    errno = 0;
    long int num = strtol(argv[1], &endptr, 10);

    if (errno == ERANGE)
    {
        printf("Number of process is out of range.\n");
        return 1;
    }
    else if (*endptr != '\0')
    {
        printf("Invalid process number.\n");
        return 1;
    }
    else
    {
        if (num < 0)
        {
            printf("Negative process number.\n");
            return 1;
        }
    }

    printf("num = %ld\n", num);
    int pid = -2;
    int status;

    for (int i = 0; i < num; ++i)
    {
        pid = fork();
        switch(pid) 
        {
        case -1: 
            printf("Fork failed.\n");
            exit(-1);
        case 0: 
            printf("Child #%d of %ld with %ld\n", i, getppid(), getpid());
            exit(0);
        default: 
            break;
        }
        wait(&status);
    }

    return 0;
}