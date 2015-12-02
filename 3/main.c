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

#define MUTEX 0
#define EMPTY 1
#define FULL 2  
#define MUTEX1 3
#define MUTEX2 4

#define BUF_SIZE 4096
#define DATA_SIZE (4096 - sizeof(int))

int main(int argc, char const *argv[])
{
    struct sembuf sop = {0};
    struct sembuf sops[2] = {{0}, {0}};
    struct semid_ds sds = {0};

    if (argc == 1) //RX
    {
        key_t key = ftok("/tmp", 1);
        int shmid = shmget(key, BUF_SIZE, 0666 | IPC_CREAT);
        char *buf = shmat(shmid, NULL, 0);

        key = ftok("/tmp", 2);
        int semid = semget(key, 5, 0666 | IPC_CREAT);

        semctl(semid, 5, IPC_STAT, &sds);
        if (sds.sem_otime == 0)
        {
            const unsigned short start_semvals[5] = {1, 1, 0, 1, 1};
            semctl(semid, 5, SETALL, start_semvals); 
        }

        sop.sem_num = MUTEX2;
        sop.sem_op = -1;
        sop.sem_flg = 0;
        semop(semid, &sop, 1);

        int print_num = 1;

        while (print_num > 0)
        {
            sops[0].sem_num = FULL;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            sops[1].sem_num = MUTEX;
            sops[1].sem_op = -1;
            sops[1].sem_flg = 0;
            semop(semid, sops, 2);

            print_num = *((int *)(buf + DATA_SIZE));
            write(1, buf, print_num);

            sops[0].sem_num = MUTEX;
            sops[0].sem_op = 1;
            sops[0].sem_flg = 0;
            sops[1].sem_num = 1;
            sops[1].sem_op = EMPTY;
            sops[1].sem_flg = 0;
            semop(semid, sops, 2);
        }

        sop.sem_num = MUTEX1;
        sop.sem_op = 1;
        sop.sem_flg = 0;
        semop(semid, &sop, 1);

        //shmctl(shmid, IPC_RMID, NULL);
        //semctl(semid, 4, IPC_RMID);
    }
    else if (argc == 2) //TX
    {   
        key_t key = ftok("/tmp", 1);
        int shmid = shmget(key, BUF_SIZE, 0666 | IPC_CREAT);
        char *buf = shmat(shmid, NULL, 0);

        key = ftok("/tmp", 2);
        int semid = semget(key, 5, 0666 | IPC_CREAT);

        int file_fd = open(argv[1], O_RDONLY);

        semctl(semid, 5, IPC_STAT, &sds);
        if (sds.sem_otime == 0)
        {
            const unsigned short start_semvals[5] = {1, 1, 0, 1, 1};
            semctl(semid, 5, SETALL, start_semvals); 
        }

        sop.sem_num = MUTEX1;
        sop.sem_op = -1;
        sop.sem_flg = 0;
        semop(semid, &sop, 1);

        int read_num = 1;

        while (read_num > 0)
        {
            sops[0].sem_num = EMPTY;
            sops[0].sem_op = -1;
            sops[0].sem_flg = 0;
            sops[1].sem_num = MUTEX;
            sops[1].sem_op = -1;
            sops[1].sem_flg = 0;
            semop(semid, sops, 2);

            read_num = read(file_fd, buf, DATA_SIZE);
            *((int *)(buf + DATA_SIZE)) = read_num;

            sops[0].sem_num = MUTEX;
            sops[0].sem_op = 1;
            sops[0].sem_flg = 0;
            sops[1].sem_num = FULL;
            sops[1].sem_op = 1;
            sops[1].sem_flg = 0;
            semop(semid, sops, 2);
        }

        sop.sem_num = MUTEX2;
        sop.sem_op = 1;
        sop.sem_flg = 0;
        semop(semid, &sop, 1);

        // shmctl(shmid, IPC_RMID, NULL);
        // semctl(semid, 4, IPC_RMID); 

        close(file_fd);
    }
    else
    {
        printf("Invalid arguments.\n");
        return -1;
    }

    return 0;
}