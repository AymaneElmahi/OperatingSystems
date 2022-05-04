#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdnoreturn.h>

#define MAXLEN 1000 // longueur maximum d'une ligne

#define NCHK(op)           \
    do                     \
    {                      \
        if ((op) == NULL)  \
        {                  \
            raler(1, #op); \
        }                  \
    } while (0)
#define TCHK(op)                \
    do                          \
    {                           \
        if ((errno = (op)) > 0) \
        {                       \
            raler(1, #op);      \
        }                       \
    } while (0)

struct arg
{
    char *fichier;          // fichier que le thread doit lire
    char ligne[MAXLEN];     // chaque ligne lue par le thread
    pthread_cond_t *cond;   // condition pour le
    pthread_mutex_t *mutex; // mutex pour le thread
    int *end_copy;          // indique si le thread doit s'arreter
    int *termine;           // indique si le thread est terminé
};

noreturn void raler(int syserr, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    if (syserr)
        perror("");

    exit(EXIT_FAILURE);
}

void *f(void *arg)
{
    struct arg *a = arg;
    char ligne[MAXLEN]; // y compris le \0 de fin de chaîne
    FILE *fp;
    pthread_cond_t *cond = a->cond;
    pthread_mutex_t *mutex = a->mutex;
    int *end_copy = a->end_copy;
    int *termine = a->termine;

    NCHK(fp = fopen(a->fichier, "r"));
    // lire la ligne dans la variable locale du thread, pendant que le thread
    // principal affiche la ligne partagée
    // recopier la ligne lue dans la variable partagée et signaler à l'autre
    while (fgets(ligne, sizeof ligne, fp) != NULL)
    {
        // attends que le thread principal ait fini d'afficher la ligne
        pthread_mutex_lock(mutex);
        while (*end_copy == 0)
        {
            pthread_cond_wait(cond, mutex);
        }
        // copie la ligne dans la variable partagée
        strcpy(a->ligne, ligne);
        // indique que le thread principal peut afficher la ligne
        *end_copy = 0;
        pthread_cond_signal(cond);
        pthread_mutex_unlock(mutex);
    }

    if (fclose(fp) == EOF)
        raler(1, "fclose %s", a->fichier);

    *termine = 1;

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t id;
    struct arg arg;
    int termine;

    if (argc != 2)
        raler(0, "usage: %s fichier", argv[0]);

    arg.fichier = argv[1];

    // initialiser la condition et le mutex
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    int end_copy = 0;
    arg.end_copy = &end_copy;
    arg.cond = &cond;
    arg.mutex = &mutex;
    arg.termine = &termine;

    TCHK(pthread_create(&id, NULL, f, &arg));

    termine = 0;
    strcpy(arg.ligne, "");

    while (!termine)
    {
        // attends que le thread fils ait fini de recopier la ligne
        pthread_mutex_lock(&mutex);
        while (end_copy == 1)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        // affiche la ligne partagée
        printf("%s", arg.ligne); // arg.ligne contient déjà \n
        // indique que le thread fils peut recopier la ligne
        end_copy = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    TCHK(pthread_join(id, NULL));

    exit(0);
}