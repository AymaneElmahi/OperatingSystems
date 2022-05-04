#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <stdnoreturn.h>
#include <stdarg.h>

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

#define MAXNOM 10

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

struct arg
{
    char nom[MAXNOM]; // le nom du client
    int duree;        // durée de la course en ms
    int *occupes;     // le tableau des livreurs occupés
    // TODO à compléter
    int id;                // id du livreur
    pthread_cond_t cond;   // condition
    pthread_mutex_t mutex; // mutex
};

void *livreur(void *arg)
{
    struct arg *a = arg;

    for (;;)
    {
        // TODO attendre une course
        pthread_mutex_lock(&a->mutex);
        while (a->nom[0] == '\0')
            pthread_cond_wait(&a->cond, &a->mutex);
        pthread_mutex_unlock(&a->mutex);

        // détecter la terminaison
        if (a->duree < 0)
            break;

        // TODO modifier le message pour inclure le numéro du livreur
        printf("Livreur %d : client %s pendant %d ms\n", a->id, a->nom, a->duree);

        usleep(a->duree * 1000); // durée de la course en micro-secondes

        // TODO réveiller le thread principal quand on a fini
        pthread_mutex_lock(&a->mutex);
        a->nom[0] = '\0';
        a->duree = -1;
        a->occupes[a->id] = 0;
        pthread_cond_signal(&a->cond);
        pthread_mutex_unlock(&a->mutex);
    }

    return NULL;
}

int trouver_livreur_pret(int *occupes, int nthreads)
{
    int i;

    for (i = 0; i < nthreads; i++)
        if (!occupes[i])
            break;

    return (i < nthreads) ? i : -1; // -1 <=> aucun livreur prêt
}

int main(int argc, char *argv[])
{
    int nthreads;     // nb de livreurs
    pthread_t *tid;   // tableau des identificateurs de threads
    struct arg *targ; // tableau des arguments
    int *occupes;     // tableau des livreurs occupés
    int i;            // compte-tours de boucle
    int liv;          // un livreur disponible
    int nval;         // nombre d'éléments lus par scanf
    char nom[MAXNOM]; // nom du client
    int duree;        // durée de la course en ms

    if (argc != 2 || (nthreads = atoi(argv[1])) <= 0)
        raler(0, "usage: %s nb-threads\n\tavec nb-threads > 0", argv[0]);

    // calloc initialise toute la mémoire à 0
    NCHK(tid = calloc(nthreads + 1, sizeof *tid));
    NCHK(occupes = calloc(nthreads, sizeof *occupes));
    NCHK(targ = calloc(nthreads + 1, sizeof *targ));

    // Créer les threads livreurs
    for (i = 0; i < nthreads; i++)
    {
        targ[i].nom[0] = '\0'; // pas de course pour le moment
        targ[i].duree = 0;     // pas vraiment nécessaire
        targ[i].occupes = occupes;
        // TODO à compléter
        targ[i].id = i;
        TCHK(pthread_mutex_init(&targ[i].mutex, NULL));
        TCHK(pthread_cond_init(&targ[i].cond, NULL));
        TCHK(pthread_create(&tid[i], NULL, livreur, &targ[i]));
    }

    // lire les courses à effectuer et les affecter aux livreurs
    while ((nval = scanf("%s%d\n", nom, &duree)) == 2)
    {
        // Tests
        if (strlen(nom) >= MAXNOM)
            raler(0, "nom '%s' trop long pour %d caractères", nom, MAXNOM);
        if (duree < 0)
            raler(0, "duree '%d' devrait être >= 0", duree);

        // TODO attendre qu'au moins un livreur soit prêt
        pthread_mutex_lock(&targ[0].mutex);
        while ((liv = trouver_livreur_pret(occupes, nthreads)) == -1)
            pthread_cond_wait(&targ[0].cond, &targ[0].mutex);
        pthread_mutex_unlock(&targ[0].mutex);

        printf("Course %s (duree = %d) -> livreur %d\n", nom, duree, liv);
        occupes[liv] = 1; // occupé => ne pas réutiliser tout de suite

        // TODO donner la course au livreur trouvé et le réveiller
        pthread_mutex_lock(&targ[liv].mutex);
        strcpy(targ[liv].nom, nom);
        targ[liv].duree = duree;
        pthread_cond_signal(&targ[liv].cond);
        pthread_mutex_unlock(&targ[liv].mutex);
    }

    if (nval != EOF)
        raler(0, "mauvais format pour <nom, duree>");

    // prévenir tous les livreurs qu'ils doivent s'arrêter
    for (i = 0; i < nthreads; i++)
    {
        // TODO le livreur i est-il occupé ? si oui, attendre
        pthread_mutex_lock(&targ[i].mutex);
        while (occupes[i])
            pthread_cond_wait(&targ[i].cond, &targ[i].mutex);
        pthread_mutex_unlock(&targ[i].mutex);

        // TODO prévenir le livreur i qu'il doit s'arrêter
        pthread_mutex_lock(&targ[i].mutex);
        strcpy(targ[i].nom, "bidon");
        targ[i].duree = -1;
        pthread_cond_signal(&targ[i].cond);
        pthread_mutex_unlock(&targ[i].mutex);

        // Attendre la terminaison du livreur i
        TCHK(pthread_join(tid[i], NULL));
    }

    free(tid);
    free(targ);
    free(occupes);

    exit(0);
}
