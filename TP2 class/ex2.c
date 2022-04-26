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

// create 9 semaphores
sem_t sem[2] = {0};

void task(int id)
{
    sleep(1);
    printf("Thread %d\n", id);
}

/**
 * @brief print the thread's id
 * @param arg the thread's id
 * @return void*
 */
void *start_routine(void *arg)
{
    int id = *(int *)arg;
    switch (id)
    {
    case 11: // allow 21 22
        task(id);
        sem_post(&sem[0]);
        sem_post(&sem[0]);
        break;
    case 12: // allow 21 22 23
        task(id);
        TCHK(sem_post(&sem[0]));
        TCHK(sem_post(&sem[0]));
        TCHK(sem_post(&sem[0]));
        break;
    case 13: // allow 23 24 31
        task(id);
        TCHK(sem_post(&sem[0]));
        TCHK(sem_post(&sem[0]));
        TCHK(sem_post(&sem[1]));
        break;
    case 21: // wait for 11 12 and allow 31
        TCHK(sem_wait(&sem[0]));
        TCHK(sem_wait(&sem[0]));
        task(id);
        TCHK(sem_post(&sem[1]));
        break;
    case 22: // wait for 11 12 and allow 32
        TCHK(sem_wait(&sem[0]));
        TCHK(sem_wait(&sem[0]));
        task(id);
        TCHK(sem_post(&sem[1]));
        break;
    case 23: // wait for 12 13 and allow 32
        TCHK(sem_wait(&sem[0]));
        TCHK(sem_wait(&sem[0]));
        task(id);
        TCHK(sem_post(&sem[1]));
        break;
    case 24: // wait for 13 and allow 32
        TCHK(sem_wait(&sem[0]));
        task(id);
        TCHK(sem_post(&sem[1]));
        break;
    case 31: // wait for 13 21
        TCHK(sem_wait(&sem[1]));
        TCHK(sem_wait(&sem[1]));
        task(id);
        break;
    case 32: // wait for 22 23 24
        TCHK(sem_wait(&sem[1]));
        TCHK(sem_wait(&sem[1]));
        TCHK(sem_wait(&sem[1]));
        task(id);
        break;
    }

    return NULL;
}

int main()
{
    // create 9 threads
    pthread_t tid[9];
    int id[9] = {11, 12, 13, 21, 22, 23, 24, 31, 32};
    for (int i = 0; i < 9; i++)
    {
        TCHK(pthread_create(&tid[i], NULL, start_routine, &id[i]));
    }
    // wait for all threads to finish
    for (int i = 0; i < 9; i++)
    {
        TCHK(pthread_join(tid[i], NULL));
    }

    return 0;
}
