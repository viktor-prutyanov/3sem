#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <errno.h>

#define HNDSHK_FIFO_NAME "0"
#define TX_PID_STR_SIZE 16

#define CLR_RED     "\x1b[31m"
#define CLR_DEFAULT "\x1b[0m"

extern int errno;

int main(int argc, char *argv[])
{
    if (argc == 1) //Receiver mode
    {
        //Handshake
        errno = 0;
        if (mkfifo(HNDSHK_FIFO_NAME, 0666) && (errno != EEXIST))
        {
            printf("FIFO '%s' creation error.\n", HNDSHK_FIFO_NAME);
            return -1;
        }

        int out_hndshk_fifo_fd = open(HNDSHK_FIFO_NAME, O_RDONLY);
        if (out_hndshk_fifo_fd == -1)
        {
            printf("FIFO opening error.\n");
            return -1;
        }

        char tx_pid_str[TX_PID_STR_SIZE] = {};
        read(out_hndshk_fifo_fd, tx_pid_str, TX_PID_STR_SIZE);
        if (tx_pid_str[0] == '\0')
        {
            printf("%sEOF received.%s\n", CLR_RED, CLR_DEFAULT);
            return -1;
        }
        //printf("{%s}\n", tx_pid_str);

        //Reception
        errno = 0;
        if (mkfifo(tx_pid_str, 0666) && (errno != EEXIST))
        {
            printf("FIFO '%s' creation error.\n", tx_pid_str);
            return -1;
        }

        int out_fifo_fd = open(tx_pid_str, O_RDONLY);
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
        close(out_hndshk_fifo_fd);
        unlink(tx_pid_str);
    }
    else if (argc == 2) //Tranceiver mode
    {   
        int file_fd = open(argv[1], O_RDONLY);
        if (file_fd == -1)
        {
            printf("File opening error.\n");
            return -1;
        }

        char tx_pid_str[TX_PID_STR_SIZE] = {};
        sprintf(tx_pid_str, "P%d", getpid());

        errno = 0;
        if (mkfifo(tx_pid_str, 0666) && (errno != EEXIST))
        {
            printf("FIFO '%s' creation error.\n", tx_pid_str);
            close(file_fd);
            return -1;
        }

        //Handshake
        errno = 0;
        if (mkfifo(HNDSHK_FIFO_NAME, 0666) && (errno != EEXIST))
        {
            printf("FIFO '%s' creation error.\n", HNDSHK_FIFO_NAME);
            return -1;
        }

        int in_hndshk_fifo_fd = open(HNDSHK_FIFO_NAME, O_WRONLY);
        if (in_hndshk_fifo_fd == -1)
        {
            printf("FIFO opening error.\n");
            return -1;
        }

        //printf("{%s}\n", tx_pid_str);
        write(in_hndshk_fifo_fd, tx_pid_str, TX_PID_STR_SIZE);
        
        int in_fifo_fd = open(tx_pid_str, O_WRONLY);
        if (in_fifo_fd == -1)
        {
            printf("FIFO opening error.\n");
            close(file_fd);
            return -1;
        }

        //Transfer

        int read_num = 1;
        char in_buffer[PIPE_BUF];
        while (read_num > 0)
        {
            read_num = read(file_fd, in_buffer, PIPE_BUF);
            write(in_fifo_fd, in_buffer, read_num);
        }

        close(file_fd);
        close(in_fifo_fd);
        close(in_hndshk_fifo_fd);
    }
    else
    {
        printf("Invalid arguments.\n");
        return -1;
    }

    return 0;
}