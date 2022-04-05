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

struct pthread_args
{
    int p;
    int t;
};

// calculate np(np+1)/2
void *start_routine(void *arg)
{
    int *u = malloc(sizeof(int));
    struct pthread_args *args = arg;
    int p = args->p;
    int t = args->t;
    // calculate sum from 1 to p of ((t-1)p +i)
    *u = 0;
    for (int i = 1; i <= p; i++)
    {
        *u += ((t - 1) * p + i);
    }
    return u;
}

/**
 * @brief create threads to calculate np(np+1)/2
 *
 * @param argc
 * @param argv n and p
 * @return int
 */
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s n p\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    int p = atoi(argv[2]);

    pthread_t *tid = calloc(sizeof(pthread_t), n);
    struct pthread_args *threads = calloc(sizeof(struct pthread_args), n);

    if (tid == NULL)
        raler(1, "malloc");
    // start n threads
    int i = 0;
    for (i = 0; i < n; i++)
    {
        threads[i].p = p;
        threads[i].t = i + 1;
        if (errno = pthread_create(&tid[i], NULL, start_routine, threads + i) > 0)
            raler(1, "pthread_create");
    }

    // wait for all threads to finish and calculate sum of return values values
    int sum = 0;
    for (i = 0; i < n; i++)
    {
        void *ret;
        if (errno = pthread_join(tid[i], &ret))
            raler(1, "pthread_join");
        sum += *(int *)ret;
        free(ret);
    }
    printf("sum = %d\n", sum);
    free(tid);
    free(threads);
    return 0;
}