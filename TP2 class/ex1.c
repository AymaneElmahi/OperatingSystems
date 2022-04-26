#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define CHK(op)            \
    do                     \
    {                      \
        if ((op) == -1)    \
            raler(1, #op); \
    } while (0)

#define TCHK(op)                \
    do                          \
    {                           \
        if ((errno = (op)) > 0) \
            raler(1, #op);      \
    } while (0)

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

// create global variable
long int compteur = 0;

// incrementes the global variable a million times
void *start_routine(void *arg)
{
    for (int i = 0; i < 1000000; i++)
    {
        compteur++;
    }
    return NULL;
}

/**
 * @brief
 *
 * @return int
 */
int main()
{
    // start timer
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // create 4 threads
    pthread_t tid[4];
    int i;
    for (i = 0; i < 4; i++)
    {
        TCHK(pthread_create(&tid[i], NULL, start_routine, NULL));
    }
    // wait for all threads to finish
    for (i = 0; i < 4; i++)
    {
        TCHK(pthread_join(tid[i], NULL));
    }
    // print the global variable
    printf("counter = %ld\n", compteur);

    // end timer
    clock_gettime(CLOCK_MONOTONIC, &end);
    long int duration = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

    // print the duration of the execution
    printf("%ld\n", duration);
    return 0;
}