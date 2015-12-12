#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

#define CHLD_BUF_SIZE 4096
#define BASE_BUF_SIZE 4096
#define BLOCK_SIZE 4096

struct pipe_fds_t
{
    int fds[2];
    char closed;
};

struct bound_t
{
    char *begin;
    char *end;
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    char *endptr = NULL;
    errno = 0;
    long int num = strtol(argv[1], &endptr, 10);

    if (errno == ERANGE)
    {
        fprintf(stderr, "Number of process is out of range.\n");
        return 1;
    }
    else if (*endptr != '\0')
    {
        fprintf(stderr, "Invalid process number.\n");
        return 1;
    }
    else
    {
        if (num <= 0)
        {
            fprintf(stderr, "Negative process number.\n");
            return 1;
        }
    }

    struct pipe_fds_t pipes[num * 2 - 1];

    for (int i = 0; i < num * 2 - 1; ++i)
    {
        pipe(pipes[i].fds);
        pipes[i].closed = 0;
    }

    int flags;
    int fcntl_ret;

    int pid = -1;
    for (int i = 0; i < num; ++i)
    {
        pid = fork();
        switch(pid) 
        {
        case -1: 
            fprintf(stderr, "Fork failed.\n");
            exit(EXIT_FAILURE);
        case 0: 
            {}
            char buf[CHLD_BUF_SIZE];    

            if (i == num - 1)
            {
                for (int j = 0; j < num * 2 - 2; ++j)
                {
                    close(pipes[j].fds[0]);
                    close(pipes[j].fds[1]);
                }
                close(pipes[2 * num - 2].fds[0]);    

                int in_fd = open(argv[2], O_RDONLY);

                int read_num = 1;
                int write_num = 0;
                while (read_num > 0)
                {
                    read_num = read(in_fd, buf, CHLD_BUF_SIZE);
                    if (read_num <= 0) break;
                    if (write(pipes[2 * num - 2].fds[1], buf, read_num) != read_num)
                    {
                        fprintf(stderr, "Head child transaction error.\n");
                        exit(EXIT_FAILURE);
                    }
                }

                close(in_fd);
                close(pipes[2 * num - 2].fds[1]);
                if (read_num == 0)
                {
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    fprintf(stderr, "File reading failed in child #%d.\n", i);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                for (int j = 0; j < 2 * i; ++j)
                {
                    close(pipes[j].fds[0]);
                    close(pipes[j].fds[1]);
                }
                close(pipes[2 * i].fds[0]);
                close(pipes[2 * i + 1].fds[1]);
                for (int j = 2 * i + 2; j < num * 2 - 1; ++j)
                {
                    close(pipes[j].fds[0]);
                    close(pipes[j].fds[1]);
                }

                int read_num = 1;
                int write_num = 0;
                while (read_num > 0)
                {
                    read_num = read(pipes[2 * i + 1].fds[0], buf, CHLD_BUF_SIZE);
                    if (read_num <= 0) break;
                    if (write(pipes[2 * i].fds[1], buf, read_num) != read_num)
                    {
                        fprintf(stderr, "Child transaction error.\n");
                        exit(EXIT_FAILURE);
                    }
                }

                close(pipes[2 * i].fds[1]);
                close(pipes[2 * i + 1].fds[0]);
                if (read_num == 0)
                {
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    fprintf(stderr, "Pipe reading failed in child #%d.\n", i);
                    exit(EXIT_FAILURE);
                }
            }
        default: 
            close(pipes[2 * i].fds[1]);
            flags = fcntl(pipes[2 * i].fds[0], F_GETFL, 0);
            fcntl(pipes[2 * i].fds[0], F_SETFL, flags | O_NONBLOCK); 
            if (i != 0)
            {
                close(pipes[2 * i - 1].fds[0]);
                flags = fcntl(pipes[2 * i - 1].fds[1], F_GETFL, 0);
                fcntl(pipes[2 * i - 1].fds[1], F_SETFL, flags | O_NONBLOCK);  
            }
            break;
        }
    }

    char *buffers[num];
    struct bound_t buf_bounds[num];
    size_t buf_size_factor = 1;

    for (int i = 0; i < num; ++i)
    {
        buffers[i] = (char *)calloc(BASE_BUF_SIZE * buf_size_factor, sizeof(char));
        buf_bounds[i].begin = buffers[i];
        buf_bounds[i].end   = buffers[i] + BASE_BUF_SIZE * buf_size_factor;
        buf_size_factor *= 3; 
    }

    fd_set read_fds;
    fd_set write_fds;

    int read_num = 0;
    int write_num = 0;

    while(1)
    {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        int max_no = -1;
        for (int i = 0; i < num; ++i)
        {
            if (!pipes[2 * i].closed)
            {
                FD_SET(pipes[2 * i].fds[0], &read_fds);
                if (pipes[2 * i].fds[0] > max_no)
                {
                    max_no = pipes[2 * i].fds[0];
                }
            }
            if ((i != 0) && (!pipes[2 * i - 1].closed))
            {
                FD_SET(pipes[2 * i - 1].fds[1], &write_fds);
                if (pipes[2 * i - 1].fds[1] > max_no)
                {
                    max_no = pipes[2 * i - 1].fds[1];
                }
            }
        }
    
        select(max_no + 1, &read_fds, &write_fds, NULL, NULL);
    
        size_t buf_engaged = buf_bounds[0].begin - buffers[0]; 
        if (buf_engaged != 0)
        {
            write_num = write(STDOUT_FILENO, buffers[0], buf_engaged);
            if (write_num != buf_engaged)
            {
                memmove(buffers[0], buffers[0] + write_num, buf_engaged - write_num); 
            }
            buf_bounds[0].begin -= write_num;
        }   
        for (int i = num - 1; i >= 0; --i)   
        {
            buf_engaged = buf_bounds[i].begin - buffers[i];
            if ((i != 0) && (buf_engaged != 0) && (FD_ISSET(pipes[2 * i - 1].fds[1], &write_fds) != 0))
            {
                size_t to_write = (buf_engaged > BLOCK_SIZE) ? BLOCK_SIZE : buf_engaged;
                write_num = write(pipes[2 * i - 1].fds[1], buffers[i], to_write);
                //fprintf(stderr, "#%d: {engaged = %lu; write_num = %d}\n", i, buf_engaged, write_num);
                if (write_num != buf_engaged)
                {
                    memmove(buffers[i], buffers[i] + write_num, buf_engaged - write_num);   
                }
                buf_bounds[i].begin -= write_num;
            }
            size_t buf_free = buf_bounds[i].end - buf_bounds[i].begin;
            if ((buf_free != 0) && (FD_ISSET(pipes[2 * i].fds[0], &read_fds) != 0))
            {
                read_num = read(pipes[2 * i].fds[0], buf_bounds[i].begin, buf_free);
                if (read_num == 0)
                {
                    close(pipes[2 * i].fds[0]);
                    pipes[2 * i].closed = 1;
                }
                else if (read_num < 0)
                {
                    fprintf(stderr, "#%d: reading error.\n", i);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    buf_bounds[i].begin += read_num;
                }
            }
        }

        for (int i = 0; i < num; ++i)
        {
            if (pipes[2 * i].closed && (buffers[i] == buf_bounds[i].begin))
            {
                if (i == 0)
                {
                    free(buffers[0]);
                    exit(EXIT_SUCCESS);
                }
                else if (!pipes[2 * i - 1].closed)
                {
                    free(buffers[i]);
                    close(pipes[2 * i - 1].fds[1]);
                    pipes[2 * i - 1].closed = 1;
                }
            }
        }
    }
    return 0;
}