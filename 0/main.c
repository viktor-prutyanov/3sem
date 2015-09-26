#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

extern int errno;

int main(int argc, char *argv[])
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
        printf("Number is out of range.\n");
        return 1;
    }
    else if (*endptr != '\0')
    {
        printf("Invalid number.\n");
        return 1;
    }
    else
    {
        if (num < 0)
        {
            printf("Negative number.\n");
            return 1;
        }
        for (long int i = 1; i < num; ++i)
        {
            printf("%ld\n", i);
        }
        printf("%ld\n", num);
        return 0;
    }

    return 0;
}