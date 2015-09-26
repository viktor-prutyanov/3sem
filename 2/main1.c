#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Invalid arguments.\n");
        return -1;
    }

    //printf("File is %s\n", argv[1]);

    int pipefds[2];
    if (pipe(pipefds))
    {
        printf("Pipe creation error.\n");
        return -1;
    }
    else
    {
        //printf("Pipe successfully created.\n");
    }

    int read_num = 0;
    int pid = -2;
    int status;
    char in_buffer[BUF_SIZE];
    char out_buffer[BUF_SIZE];
    pid = fork();

    switch(pid) 
    {
    case -1: 
        printf("Fork failed.\n");
        exit(-1);
    case 0: //Child
        //printf("Child of pid=%ld with pid=%ld created.\n", getppid(), getpid());

        if (close(pipefds[0]))
        {
            printf("Pipe output closing error.\n");
            exit(-1);
        }
        else
        {
            //printf("Useless pipe output closed by child.\n");
        }

        int input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1)
        {
            printf("Opening error.\n");
            exit(-1);
        }

        read_num = 1;

        while (read_num > 0)
        {
            read_num = read(input_fd, in_buffer, BUF_SIZE);
            write(pipefds[1], in_buffer, read_num);
        }

        close(input_fd);
        close(pipefds[1]);
        exit(0);
    default: //Parent
        if (close(pipefds[1]))
        {
            printf("Pipe input closing error.\n");
            exit(-1);
        }
        else
        {
            //printf("Useless pipe input closed by parent.\n");
        }
        //printf("Your file content is:\n");

        read_num = 1;

        while (read_num > 0)
        {
            read_num = read(pipefds[0], out_buffer, BUF_SIZE);
            write(1, out_buffer, read_num);
        }

        close(pipefds[0]);
        break;
    }
    wait(&status);
}