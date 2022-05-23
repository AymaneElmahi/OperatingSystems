#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

/*
 * Compiler avec -DINTERBLOCAGE pour voir l'interblocage se produire
 *	(visible seulement avec un très grand nombre de repas, par ex 10000)
 * Compiler avec -DTERMINAISON pour que table attende que le dernier
 *	philosophe ait quitté la table
 * Compiler sans -D pour une version sans interblocage et sans attente
 *	de terminaison
 */

/*
 * Il faudrait mettre la définition dans un fichier .h séparé pour mutualiser
 * avec table.c
 */

#define MEMNAME "/table"
struct mem
{
    // partie fixe : unique pour tous les philosophes
    int nphilo;
    int narrives;
    sem_t mtx; // exclusion mutuelle (init = 1)
#if defined(TERMINAISON)
    int ntermines; // nb de philosophes terminés
    sem_t fin;     // attente de la fin (init = 0)
#endif

    // partie variable : il y a un élément par philosophe (ou plus
    // exactement par baguette)
    // Note : les tableaux sans dimension sont autorisés à la fin des
    // structures, sizeof renvoie alors la taille sans cet élément
    sem_t tabsem[]; // baguette (init = 1)
};

#define CHK(op)            \
    do                     \
    {                      \
        if ((op) == -1)    \
            raler(1, #op); \
    } while (0)

noreturn void raler(int syserr, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    if (syserr)
        perror("");

    exit(1);
}

void usage(char *prog)
{
    raler(0, "usage: %s nrepas", prog);
}

int main(int argc, char *argv[])
{
    int fd, i, moi, nrepas;
    off_t sz;
    struct mem *m;
    struct stat stbuf;

    if (argc != 2)
        usage(argv[0]);

    nrepas = atoi(argv[1]);
    if (nrepas <= 0)
        usage(argv[0]);

    CHK(fd = shm_open(MEMNAME, O_RDWR, 0));
    CHK(fstat(fd, &stbuf));
    sz = stbuf.st_size;

    m = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (m == MAP_FAILED)
        raler(1, "cannot mmap");
    CHK(close(fd)); // on n'a plus besoin de l'ouverture

    CHK(sem_wait(&m->mtx));
    moi = m->narrives++; // ++ sur var partagée => section critique
    CHK(sem_post(&m->mtx));

    moi %= m->nphilo; // pour pouvoir redémarrer les philosophes

    for (i = 0; i < nrepas; i++)
    {
#if defined(INTERBLOCAGE)
        CHK(sem_wait(&m->tabsem[moi]));
        CHK(sem_wait(&m->tabsem[(moi + 1) % m->nphilo]));
#else
        if (moi == 0) // il suffit qu'un des philosophes soit droitier
        {
            CHK(sem_wait(&m->tabsem[(moi + 1) % m->nphilo]));
            CHK(sem_wait(&m->tabsem[moi]));
        }
        else // alors que les autres sont gauchers
        {
            CHK(sem_wait(&m->tabsem[moi]));
            CHK(sem_wait(&m->tabsem[(moi + 1) % m->nphilo]));
        }
#endif
        printf("%d : je mange\n", moi);
        fflush(stdout);

        // les libérations n'ont pas besoin d'être ordonnées
        CHK(sem_post(&m->tabsem[moi]));
        CHK(sem_post(&m->tabsem[(moi + 1) % m->nphilo]));
    }

#if defined(TERMINAISON)
    CHK(sem_wait(&m->mtx));
    m->ntermines++;
    if (m->ntermines == m->nphilo)
        CHK(sem_post(&m->fin)); // réveiller la table
    CHK(sem_post(&m->mtx));
#endif

    exit(0);
}