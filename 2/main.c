#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Invalid arguments.\n");
        return -1;
    }

    if (mkfifo("FIFO_1", 0666))
    {
        printf("Fifo creation error.\n");
        return -1;
    }

    int read_num = 0;
    int pid = -2;
    int status;
    char in_buffer[BUF_SIZE];
    char out_buffer[BUF_SIZE];
    pid = fork();
    int in_fifo_fd, out_fifo_fd, in_file_fd;

    switch(pid) 
    {
    case -1: 
        printf("Fork failed.\n");
        exit(-1);
    case 0: //Child
        in_file_fd = open(argv[1], O_RDONLY);
        if (in_file_fd == -1)
        {
            printf("Opening error.\n");
            exit(-1);
        }

        in_fifo_fd = open("FIFO_1", O_WRONLY);
        if (in_fifo_fd == -1)
        {
            printf("FIFO opening error.\n");
            exit(-1);
        }

        read_num = 1;

        while (read_num > 0)
        {
            read_num = read(in_file_fd, in_buffer, BUF_SIZE);
            write(in_fifo_fd, in_buffer, read_num);
        }

        close(in_file_fd);
        close(in_fifo_fd);
        exit(0);
    default: //Parent
        out_fifo_fd = open("FIFO_1", O_RDONLY);
        if (out_fifo_fd == -1)
        {
            printf("FIFO opening error.\n");
            exit(-1);
        }

        read_num = 1;

        while (read_num > 0)
        {
            read_num = read(out_fifo_fd, out_buffer, BUF_SIZE);
            write(1, out_buffer, read_num);
        }

        close(out_fifo_fd);
        break;
    }
    wait(&status);
    unlink("FIFO_1");
}