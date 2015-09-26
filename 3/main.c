#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <errno.h>

extern int errno;

int main(int argc, char *argv[])
{
    if (argc == 1) //Receiver mode
    {
        int out_fifo_fd = open("FIFO_1", O_RDONLY);
        if (out_fifo_fd == -1)
        {
            printf("FIFO opening error.\n");
            return -1;
        }

        int read_num = 1;
        char out_buffer[PIPE_BUF];

        while (read_num > 0)
        {
            read_num = read(out_fifo_fd, out_buffer, PIPE_BUF);
            write(1, out_buffer, read_num);
        }

        close(out_fifo_fd);
    }
    else if (argc == 2) //Tranceiver mode
    {   
        int in_file_fd = open(argv[1], O_RDONLY);
        if (in_file_fd == -1)
        {
            printf("File opening error.\n");
            return -1;
        }

        errno = 0;
        if (mkfifo("FIFO_1", 0666) && (errno != EEXIST))
        {
            printf("FIFO creation error.\n");
            close(in_file_fd);
            return -1;
        }

        int in_fifo_fd = open("FIFO_1", O_WRONLY);
        if (in_fifo_fd == -1)
        {
            printf("FIFO opening error.\n");
            close(in_file_fd);
            return -1;
        }

        int read_num = 1;
        char in_buffer[PIPE_BUF];

        while (read_num > 0)
        {
            read_num = read(in_file_fd, in_buffer, PIPE_BUF);
            write(in_fifo_fd, in_buffer, read_num);
        }

        close(in_file_fd);
        close(in_fifo_fd);
    }
    else
    {
        printf("Invalid arguments.\n");
    }
}