// Fichier lib.h à rédiger
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
#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */

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
noreturn void raler(int syserr, const char *msg, ...);
int is_empty(int fd);

struct produit
{
    char nom[20];
    int quantite;
};

// set the named semaphore function

sem_t *set_sem(char *name, int value);
