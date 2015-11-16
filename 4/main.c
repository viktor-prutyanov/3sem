#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/prctl.h>

char buf = 0;
int counter = 0;
pid_t ppid = 0;
pid_t pid = 0;

void one(int signo)
{
    buf |= (1 << counter);
    ++counter;
    if (counter == 8)
    {
        printf("%c", buf);
        buf = 0;
        counter = 0;
    }
}

void zero(int signo)
{
    ++counter;
    if (counter == 8)
    {
        printf("%c", buf);
        buf = 0;
        counter = 0;
    }
}

void child_death(int signo)
{
    fprintf(stderr, "Exit because of child death.\n");
    exit(EXIT_SUCCESS);
}

void next(int signo) {

}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Invalid arguments.\n");
        return EXIT_FAILURE;
    }
    
    struct sigaction one_action = {};
    one_action.sa_handler = one;
    sigfillset(&one_action.sa_mask);
    sigaction(SIGUSR1, &one_action, NULL);

    struct sigaction zero_action = {};
    zero_action.sa_handler = zero;
    sigfillset(&zero_action.sa_mask);
    sigaction(SIGUSR2, &zero_action, NULL);
  
    struct sigaction child_death_action = {};
    child_death_action.sa_handler = child_death;
    sigfillset(&child_death_action.sa_mask);
    sigaction(SIGCHLD, &child_death_action, NULL);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);
    sigemptyset(&set);

    ppid = getpid();
    pid = fork();

    switch(pid)
    {
    case -1:
        fprintf(stderr, "Fork failed.\n");
        exit(EXIT_FAILURE);
    case 0: //Child
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        sigemptyset(&set);

        int in_file_fd = open(argv[1], O_RDONLY);
        if (in_file_fd == -1)
        {
            fprintf(stderr, "Opening error.\n");
            exit(EXIT_FAILURE);
        }

        struct sigaction next_action = {};
        next_action.sa_handler = next;
        sigfillset(&next_action.sa_mask);    
        sigaction(SIGUSR1, &next_action, NULL);

        int read_num = 1;

        while (read_num > 0)
        {
            read_num = read(in_file_fd, &buf, 1);
            if (read_num != 0)
            {
                for (int i = 0; i < 8; ++i)
                {
                    buf % 2 == 0 ? kill(ppid, SIGUSR2) : kill(ppid, SIGUSR1); 
                    buf = buf >> 1;
                    sigsuspend(&set);
                }
            }
        }
        close(in_file_fd);
        exit(EXIT_SUCCESS);
    default: //Parent
        while(1)
        {
            sigsuspend(&set);
            kill(pid, SIGUSR1);
        }
        exit(EXIT_SUCCESS);
    }
    return EXIT_SUCCESS;
}
