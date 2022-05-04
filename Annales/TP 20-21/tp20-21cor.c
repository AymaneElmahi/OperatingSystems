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

/*
 * Principe des synchronisations mises en place :
 *
 * Il y a deux points critiques :
 * - la ligne lue, partagée entre les deux threads
 * - la terminaison, signalée par le thread fils (TF) et utilisée
 *	par le thread principal (TP)
 *
 * La ligne ne peut être écrite par TF que lorsque TF ne l'utilise
 * pas. TP ne peut l'utiliser que si TF ne la remplit pas.
 * En conséquence, il faut des conditions POSIX pour synchroniser
 * les threads :
 * - une condition POSIX pour que TF indique à TP qu'il peut afficher
 *	la ligne. Cette condition doit être accompagnée d'une variable
 *	pour indiquer que la ligne est prête
 * - une condition POSIX pour que TP indique à TF qu'il a fini d'afficher
 *	la ligne et que TF peut remplir à nouveau la ligne. De même,
 *	une variable est nécessaire pour identifier cette assertion.
 * Ces deux conditions sont mutuellement exclusives (les threads ne
 * peuvent pas être mutuellement en attente pour le même événement,
 * on peut utiliser la même condition POSIX, ainsi que la même variable
 * (appelée ici "libre").
 * De plus, la condition sur laquelle TP attend que TF se termine
 * est forcément la même que la condition sur laquelle TP attend que
 * TF remplisse la ligne : un thread ne peut pas attendre simultanément
 * deux conditions différentes. On complète donc par une deuxième
 * variable (appelée ici "termine") pour que TF signale que l'événement
 * arrivé est distinct.
 *
 * À la fin, on a une condition "cnd", et donc un mutex "mtx,
 * et deux variables "libre" et "termine".
 */

struct arg
{
    char *fichier;      // fichier que le thread doit lire
    char ligne[MAXLEN]; // chaque ligne lue par le thread
    // TODO : il faut sans doute d'autres champs, en particulier pour
    // les mécanismes de synchronisation...
    int libre;   // 1: ligne "libre", 0: ligne = ligne lue
    int termine; // 1: terminé, 0: pas terminé
    pthread_cond_t cnd;
    pthread_mutex_t mtx;
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

    NCHK(fp = fopen(a->fichier, "r"));
    // lire la ligne dans la variable locale du thread, pendant que le thread
    // principal affiche la ligne partagée
    while (fgets(ligne, sizeof ligne, fp) != NULL)
    {
        // TODO : est-ce qu'on peut écrire dans la variable partagée ?
        TCHK(pthread_mutex_lock(&a->mtx));
        while (a->libre == 0)
            TCHK(pthread_cond_wait(&a->cnd, &a->mtx));
        TCHK(pthread_mutex_unlock(&a->mtx));

        // recopier la ligne lue dans la variable partagée
        strcpy(a->ligne, ligne);
        // TODO : prévenir le thread principal qu'il peut utiliser la ligne
        TCHK(pthread_mutex_lock(&a->mtx));
        a->libre = 0;
        TCHK(pthread_cond_signal(&a->cnd));
        TCHK(pthread_mutex_unlock(&a->mtx));
    }
    // TODO : prévenir le thread principal que c'est terminé
    TCHK(pthread_mutex_lock(&a->mtx));
    // le thread principal n'a peut-être pas encore affiché la dernière ligne
    while (a->libre == 0)
        TCHK(pthread_cond_wait(&a->cnd, &a->mtx));
    a->termine = 1;
    TCHK(pthread_cond_signal(&a->cnd));
    TCHK(pthread_mutex_unlock(&a->mtx));

    if (fclose(fp) == EOF)
        raler(1, "fclose %s", a->fichier);

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
    arg.libre = 1;
    arg.termine = 0;
    // l'initialisation des mécanismes de synchro doit être réalisé
    // avant la création du thread fils
    TCHK(pthread_mutex_init(&arg.mtx, NULL));
    TCHK(pthread_cond_init(&arg.cnd, NULL));

    TCHK(pthread_create(&id, NULL, f, &arg));

    termine = 0;
    while (!termine)
    {
        // TODO : est-ce qu'on a terminé ?
        // TODO : peut-on déjà utiliser la ligne lue par le thread ?
        TCHK(pthread_mutex_lock(&arg.mtx));
        while (arg.libre && !arg.termine)
            TCHK(pthread_cond_wait(&arg.cnd, &arg.mtx));
        TCHK(pthread_mutex_unlock(&arg.mtx));

        /*
         * Quand on arrive ici, soit la place n'est plus libre
         * (et on peut afficher la chaîne qui s'y trouve), soit
         * la lecture est terminée
         */

        if (arg.termine)
            termine = 1;
        else
        {

            printf("%s", arg.ligne); // arg.ligne contient déjà \n
            // TODO : prévenir le thread fils qu'il peut à nouveau utiliser la ligne
            TCHK(pthread_mutex_lock(&arg.mtx));
            arg.libre = 1;
            TCHK(pthread_cond_signal(&arg.cnd));
            TCHK(pthread_mutex_unlock(&arg.mtx));
        }
    }

    TCHK(pthread_join(id, NULL));
    TCHK(pthread_mutex_destroy(&arg.mtx));
    TCHK(pthread_cond_destroy(&arg.cnd));

    exit(0);
}
