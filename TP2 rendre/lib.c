// Fichier lib.c à rédiger
#include "lib.h"

noreturn void raler(int syserr, const char *msg, ...)
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

int is_empty(int fd)
{
    struct stat s;
    CHK(fstat(fd, &s));
    return s.st_size == 0;
}

sem_t *set_sem(char *name, int value)
{
    sem_t *sem;
    sem = sem_open(name, O_CREAT, 0666, value);
    NCHK(sem);
    return sem;
}
