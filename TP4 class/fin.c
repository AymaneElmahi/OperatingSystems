#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <stdnoreturn.h>
#include <stdarg.h>
#include <sys/mman.h>

#define CHK(op)            \
    do                     \
    {                      \
        if ((op) == -1)    \
            raler(1, #op); \
    } while (0)
#define NCHK(op)           \
    do                     \
    {                      \
        if ((op) == NULL)  \
            raler(1, #op); \
    } while (0)
#define TCHK(op)                \
    do                          \
    {                           \
        if ((errno = (op)) > 0) \
            raler(1, #op);      \
    } while (0)

noreturn void
raler(int syserr, const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    if (syserr == 1)
        perror("");

    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        raler(0, "Usage: %s <file>", argv[0]);

    int fd;
    char *tab;
    CHK(fd = open(argv[1], O_RDONLY));

    struct stat s;
    CHK(fstat(fd, &s));

    tab = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (tab == MAP_FAILED)
        raler(1, "mmap");

    CHK(close(fd));

    int i;

    for (i = s.st_size - 1; i >= 0; i--)
    {
        if (tab[i] == '\n')
            break;
    }

    write(1, tab + i + 1, s.st_size - i - 1);
}