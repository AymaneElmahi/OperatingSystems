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
#include <semaphore.h>

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
        railer(0, "Usage: %s <nphilo>", argv[0]);

    // initialize segment of shared memory containing the number of philosophers, the chopsticks, and the philosophers
    int fd;
    CHK(fd = shm_open("/table", O_RDWR | O_CREAT | O_TRUNC, 0666));
    CHK(ftruncate(fd, sizeof(int) + sizeof(int) + sizeof(sem_t) * atoi(argv[1])));

    // map the shared memory segment into the address space of the process
    void *shared_memory = mmap(NULL, sizeof(int) + sizeof(int) + sizeof(sem_t) * atoi(argv[1]), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    NCHK(shared_memory);

    // initialize the number of philosophers
    *(int *)shared_memory = atoi(argv[1]);

    // initialize the number of chopsticks
    *(int *)shared_memory = atoi(argv[1]);

    // initialize the semaphores
    for (int i = 0; i < atoi(argv[1]); i++)
        sem_init(&((sem_t *)shared_memory)[i], 1, 1);

    // close the shared memory segment
    CHK(close(fd));

    // detach the shared memory segment
    CHK(shm_unlink("/table"));

    return 0;
}
