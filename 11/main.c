#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

struct message {
    long type;
    char text;
};

int main(int argc, char const *argv[])
{    
    if (argv[1][0] == '0') //TX
    {
        printf("TX\n");
        key_t key = ftok("/tmp", 255);
        errno = 0;
        int msqid = msgget(key, IPC_CREAT | 0666);
        perror(NULL);
        struct message msg0 = {1l, 15};
        errno = 0;
        msgsnd(msqid, &msg0, sizeof(msg0.text), 0666);
        perror(NULL);
        return 0;
    }
    else //RX
    {
        printf("RX\n");
        key_t key = ftok("/tmp", 255);
        errno = 0;
        int msqid = msgget(key, IPC_CREAT);
        perror(NULL);
        struct message msg1 = {0l, 0};
        errno = 0;
        msgrcv(msqid, &msg1, sizeof(msg1.text), 0, 0666);
        perror(NULL);
        printf("%d\n", msg1.text);
        return 0;
    }
	return 0;
}
