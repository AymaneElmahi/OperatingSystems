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
#include <string.h>

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

struct pthread_thread_arg
{
    int p;
    int n;
    int i;
    char **string;
    pthread_barrier_t *bar;
};

// if p even write "#" in string[i] else write "-"
// wait for every thread to finish using barrier
void *start_routine(void *arg)
{
    struct pthread_thread_arg *thread_arg = arg;
    int p = thread_arg->p;
    int n = thread_arg->n;
    int i = thread_arg->i;
    char **string = thread_arg->string;

    for (int j = 0; j < p; j++)
    {
        if (p % 2 == 0)
        {
            *string[i] = '#';
        }
        else
        {
            *string[i] = '-';
        }
        // wait for all threads to finish using barrier and print string
        pthread_barrier_wait(thread_arg->bar);
        if (i == n - 1)
            printf("%s\n", *string);
    }
    return NULL;
}

/**
 * @brief write alternatively n times "#"" and n times "-"" p times
 *
 * @param argc
 * @param argv n and p
 * @return int 0
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
    char *string = calloc(sizeof(char), n);
    if (string == NULL)
        raler(1, "malloc");

    pthread_t *tid = calloc(n, sizeof(pthread_t));
    if (tid == NULL)
        raler(1, "malloc");

    struct pthread_thread_arg *threads = calloc(n, sizeof(struct pthread_thread_arg));
    if (threads == NULL)
        raler(1, "malloc");

    // initialize barrier
    pthread_barrier_t bar;
    pthread_barrier_init(&bar, NULL, n);

    for (int i = 0; i < n; i++)
    {
        threads[i].p = p;
        threads[i].n = n;
        threads[i].i = i;
        // threads[i].string is the pointer to string
        threads[i].string = &string;
        threads[i].bar = &bar;

        if ((errno = pthread_create(&tid[i], NULL, start_routine, &threads[i])) > 0)
        {
            raler(1, "pthread_create");
        }
    }
    free(tid);
    free(threads);
    // destroy barrier
    pthread_barrier_destroy(&bar);
    return 0;
}
