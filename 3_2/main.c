#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <linux/limits.h>
#include <errno.h>

#define SEM0 0
#define SEMT 1
#define SEMR 2

#define BUF_SIZE 4096
#define DATA_SIZE (4096 - sizeof(int))

#define OP(op_num, num, op, flg)\
    sops[op_num].sem_num = num; \
    sops[op_num].sem_op  = op;  \
    sops[op_num].sem_flg = flg;

int main(int argc, char const *argv[])
{
    struct sembuf sops[4] = {{}, {}, {}, {}};

    if (argc == 1) //RX
    {
        key_t key = ftok("/tmp", 1);
        int shmid = shmget(key, BUF_SIZE, 0666 | IPC_CREAT);
        char *buf = shmat(shmid, NULL, 0);

        key = ftok("/tmp", 2);
        int semid = semget(key, 3, 0666 | IPC_CREAT);

        OP(0, SEM0,  1, SEM_UNDO);
        OP(1, SEM0, -1, 0);
        OP(2, SEMR, 0, 0);
        OP(3, SEMR, 1, SEM_UNDO);
        semop(semid, sops, 4);
        
        int print_num = 1;

        OP(0, SEMT, -1, 0);
        OP(1, SEMT,  1, 0);
        OP(2, SEM0, -1, 0);
        OP(3, SEM0,  1, 0);
        semop(semid, sops, 4);

        print_num = *((int *)(buf + DATA_SIZE));
        write(1, buf, print_num);

        OP(0, SEM0, -1, 0);
        semop(semid, sops, 1);

        while (print_num > 0)
        {
            OP(0, SEMT, -1, IPC_NOWAIT);
            OP(1, SEMT,  1, 0);
            OP(2, SEM0, -1, 0);
            OP(3, SEM0,  1, 0);
            errno = 0;
            semop(semid, sops, 4);
            if (errno == EAGAIN)
            {
                fprintf(stderr, "TX dead.\n");
                exit(EXIT_FAILURE);
            }

            print_num = *((int *)(buf + DATA_SIZE));
            write(1, buf, print_num);

            OP(0, SEM0, -1, 0);
            semop(semid, sops, 1);
        }
        
        OP(0, SEMR, -1, 0);
        semop(semid, sops, 1); 

        //shmctl(shmid, IPC_RMID, NULL);
        //semctl(semid, 4, IPC_RMID);
    }
    else if (argc == 2) //TX
    {   
        key_t key = ftok("/tmp", 1);
        int shmid = shmget(key, BUF_SIZE, 0666 | IPC_CREAT);
        char *buf = shmat(shmid, NULL, 0);

        key = ftok("/tmp", 2);
        int semid = semget(key, 3, 0666 | IPC_CREAT);

        int file_fd = open(argv[1], O_RDONLY);

        OP(0, SEMT, 0, 0);
        OP(1, SEMT, 1, SEM_UNDO);
        semop(semid, sops, 2);

        int read_num = 1;

        OP(0, SEMR, -1, 0);
        OP(1, SEMR,  1, 0);
        OP(2, SEM0,  0, 0);
        semop(semid, sops, 3);
        
        read_num = read(file_fd, buf, DATA_SIZE);
        *((int *)(buf + DATA_SIZE)) = read_num;

        OP(0, SEM0, 1, 0);
        semop(semid, sops, 1);

        while (read_num > 0)
        {
            OP(0, SEMR, -1, IPC_NOWAIT);
            OP(1, SEMR,  1, 0);
            OP(2, SEM0,  0, 0);
            errno = 0;
            semop(semid, sops, 3);
            if (errno == EAGAIN)
            {
                fprintf(stderr, "RX dead.\n");
                exit(EXIT_FAILURE);
            }

            read_num = read(file_fd, buf, DATA_SIZE);
            *((int *)(buf + DATA_SIZE)) = read_num;

            OP(0, SEM0, 1, 0);
            semop(semid, sops, 1);
        }
        
        OP(0, SEMT, -1, 0);
        semop(semid, sops, 1); 
        
        close(file_fd);
    }
    else
    {
        printf("Invalid arguments.\n");
        return -1;
    }

    return 0;
}
