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
#include <semaphore.h>

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

/**************************************************************************/

struct pthread_thread_arg
{
    int id;
    int m;
    int p;
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;
};

int engines_ready = 0;

void *start_routine(void *arg)
{
    struct pthread_thread_arg *thread_arg = arg;
    int id = thread_arg->id;
    int m = thread_arg->m;
    int p = thread_arg->p;
    pthread_cond_t *cond = thread_arg->cond;
    pthread_mutex_t *mutex = thread_arg->mutex;

    unsigned int seed = 0;

    for (int j = 1; j <= p; j++)
    {
        // pick a random number between 1 and m with thread safe rand()
        int k = rand_r(&seed) % m + 1;

        pthread_mutex_lock(mutex);
        // wait for k engines to be ready
        while (engines_ready < k)
        {
            TCHK(pthread_cond_wait(cond, mutex));
        }
        engines_ready -= k;

        pthread_mutex_unlock(mutex);
        // do job
        printf("%d starts job %d\n", id, j);
        int t = rand_r(&seed) % 3 + 1;
        sleep(t);

        // signal k engines that they are ready
        pthread_mutex_lock(mutex);

        TCHK(pthread_cond_broadcast(cond));
        engines_ready += k;
        pthread_mutex_unlock(mutex);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // check if there is 3 arguments m,n and p
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s m n p\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int p = atoi(argv[3]);

    engines_ready = m;

    // create n thread id and n structs
    pthread_t *tid = calloc(n, sizeof(pthread_t));
    if (tid == NULL)
    {
        raler(1, "malloc");
    }
    struct pthread_thread_arg *threads = calloc(n, sizeof(struct pthread_thread_arg));
    if (threads == NULL)
    {
        raler(1, "malloc");
    }

    // initialize condition and mutex
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    // create n threads
    for (int i = 0; i < n; i++)
    {
        threads[i].id = i;
        threads[i].m = m;
        threads[i].p = p;
        threads[i].cond = &cond;
        threads[i].mutex = &mutex;
        TCHK(pthread_create(&tid[i], NULL, start_routine, &threads[i]));
    }
    // wait for all threads to finish
    for (int i = 0; i < n; i++)
    {
        TCHK(pthread_join(tid[i], NULL));
    }
    free(tid);
    free(threads);
    return 0;
}