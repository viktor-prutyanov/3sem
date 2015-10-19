#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <linux/limits.h>
#include <errno.h>

struct message {
    long int type;
    char text;
};

int main(int argc, char const *argv[])
{
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

    errno = 0;
    int msqid = msgget(IPC_PRIVATE, 0666);
    //perror(NULL);
    struct message msg = {0l, 1};

    int pid;

    for (int i = 1; i <= num; ++i)
    {
        pid = fork();
        switch(pid) 
        {
        case -1: 
            printf("Fork failed.\n");
            
            exit(-1);
        case 0: 
            errno = 0;
            msgrcv(msqid, &msg, sizeof(msg.text), i, 0666);
            //perror(NULL);
            printf("%ld\n", msg.type);
            if (msg.type == num)
            {
                printf("Last!\n");
                errno = 0;
                msgctl(msqid, IPC_RMID, NULL);
                exit(0);    
            }
            ++msg.type;
            errno = 0;
            msgsnd(msqid, &msg, sizeof(msg.text), 0666);
            //perror(NULL);
            exit(0);
        default: 
            break;
        }
    }

    msg.type = 1;
    errno = 0;
    msgsnd(msqid, &msg, sizeof(msg.text), 0666);
    //perror(NULL);

    return 0;
}