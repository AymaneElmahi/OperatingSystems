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

long int counter = 0;
// create mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief incrementes a counter a million times
 *
 * @param arg
 * @return void*
 */
void *start_routine(void *arg)
{

    for (int i = 0; i < 1000000; i++)
    {

        CHK(pthread_mutex_lock(&mutex));
        counter++;
        CHK(pthread_mutex_unlock(&mutex));
    }

    return NULL;
}

/**
 * @brief create 4 threads and use mutex to synchronize them and get duration
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
    // start the threads
    for (i = 0; i < 4; i++)
    {
        TCHK(pthread_create(&tid[i], NULL, start_routine, NULL));
    }
    // wait for the threads to finish
    for (i = 0; i < 4; i++)
    {
        TCHK(pthread_join(tid[i], NULL));
    }
    printf("counter = %ld\n", counter);

    // end timer
    clock_gettime(CLOCK_MONOTONIC, &end);
    long int duration = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

    // print the duration of the execution
    printf("%ld\n", duration);
    return 0;
}