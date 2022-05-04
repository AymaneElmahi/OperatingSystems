#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdnoreturn.h>
#include <stdarg.h>
#include <pthread.h>
#include <errno.h>

/*
 * Des macros bien pratiques pour alléger les tests d'erreur.
 */

#define TCHK(op)                 \
    do                           \
    {                            \
        if ((errno = (op)) != 0) \
            raler(1, #op);       \
    } while (0)

#define MCHK(op)           \
    do                     \
    {                      \
        if ((op) == NULL)  \
            raler(1, #op); \
    } while (0)

// la fonction habituelle
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

// affiche un message d'erreur
noreturn void usage(void)
{
    raler(0, "usage: prodcons <bufsize> <nconso> <niter><car> ... <niter><car>");
}

// analyse une spécification de producteur ("100x" par ex)
// arg : chaîne de caractères en entrée
// pn : adresse d'une variable pour recevoir en sortie le nombre
// pc : adresse d'une variable pour recevoir en sortie le caractère
void decode_producteur(const char *arg, long int *pn, unsigned char *pc)
{
    char *end;
    long int n;

    errno = 0;
    n = strtol(arg, &end, 10);
    if (errno != 0)
        raler(1, "cannot parse '%s'", arg);
    if (arg == end)
        raler(0, "cannot parse number '%s'", arg);
    if (strlen(end) != 1)
        raler(0, "cannot parse char '%s' in '%s'", end, arg);
    if (n < 0)
        raler(0, "'%s': count must be >= 0", arg);
    *pn = n;
    *pc = *end;
}

/******************************************************************************
Main
******************************************************************************/

int main(int argc, char *argv[])
{
    int bufsize;
    int nconso;

    if (argc < 4)
        usage();

    bufsize = atoi(argv[1]);
    if (bufsize <= 0)
        usage();

    nconso = atoi(argv[2]);
    if (nconso <= 0)
        usage();

    // création du buffer
    unsigned char *buf = malloc((argc - 3) * sizeof(int *));
    if (buf == NULL)
        raler(1, "cannot allocate buffer");

    for (int i = 0; i < argc - 3; i++)
    {
        buf = malloc(2 * sizeof(int));
        if (buf == NULL)
            raler(1, "cannot allocate buffer");
        decode_producteur(argv[i + 3], &buf[0], &buf[1]);
    }
    exit(0);
}