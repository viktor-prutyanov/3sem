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

#define CLR_RED     "\x1b[31m"
#define CLR_DEFAULT "\x1b[0m"

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

    key_t key = ftok(argv[0], getpid());

    errno = 0;
    int msqid = msgget(key, IPC_CREAT | 0666);
    if (errno != 0)
    {
        perror(NULL);
        return 1;
    }
    struct message msg = {0l, 0};
    int pid;

    for (int i = 1; i <= num; ++i)
    {
        pid = fork();
        switch(pid) 
        {
        case -1: 
            printf("%sFork #%d has failed.\n%s", CLR_RED, i, CLR_DEFAULT);
            // for (int j = 1; j < i; ++j)
            // {
            //     msg.type = j;
            //     msg.text = 1;
            //     errno = 0;
            //     msgsnd(msqid, &msg, sizeof(msg.text), 0666);
            //     if (errno != 0)
            //     {
            //         perror(NULL);
            //         break;
            //     }
            // }
            errno = 0;
            if (msgctl(msqid, IPC_RMID, NULL) != 0)
                perror(NULL);
            exit(1);
        case 0: 
            errno = 0;
            msgrcv(msqid, &msg, sizeof(msg.text), i, 0666);
            if (errno != 0)
            {
                perror(NULL);
                errno = 0;
                if (msgctl(msqid, IPC_RMID, NULL) != 0)
                    perror(NULL);
                exit(1);
            }

            if (msg.text != 0) //One of forks has failed
            {
                exit(1);
            }

            fprintf(stderr, "%ld ", msg.type);

            ++msg.type;
            errno = 0;
            msgsnd(msqid, &msg, sizeof(msg.text), 0666);
            if (errno != 0)
            {
                perror(NULL);
                errno = 0;
                if (msgctl(msqid, IPC_RMID, NULL) != 0)
                    perror(NULL);
                exit(1);
            }

            exit(0);
        default: 
            break;
        }
    }

    msg.type = 1;
    errno = 0;
    msgsnd(msqid, &msg, sizeof(msg.text), 0666);
    if (errno != 0)
    {
        perror(NULL);
        errno = 0;
        if (msgctl(msqid, IPC_RMID, NULL) != 0)
            perror(NULL);
        exit(1);
    }

    msgrcv(msqid, &msg, sizeof(msg.text), num + 1, 0666);
    if (msgctl(msqid, IPC_RMID, NULL) != 0)
        perror(NULL);

    return 0;
}
