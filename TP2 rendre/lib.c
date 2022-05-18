// Fichier lib.c à rédiger
#include "lib.h"

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

int is_empty(int fd)
{
    struct stat s;
    CHK(fstat(fd, &s));
    return s.st_size == 0;
}

sem_t *set_sem(sem_t **sem, char *name, int value)
{
    *sem = sem_open(name, O_CREAT, 0666, value);
    NCHK(*sem);
    return *sem;
}

sem_t *set_sem_file(sem_t **sem, char *name, int value)
{
    // add _file to the name
    char *name_file = malloc(strlen(name) + 5);
    strcpy(name_file, name);
    strcat(name_file, "_file");
    *sem = sem_open(name_file, O_CREAT, 0666, value);
    NCHK(*sem);
    free(name_file);
    return *sem;
}

int group_products(struct produit *products, char *argv[], int nb)
{
    if (nb == 3) // it means we only had one product at the beggining
    {
        return 1;
    }
    int nb_prod = 1;
    for (int i = 3; i < nb; i += 2)
    {
        // check if argv[i] is already in products, if not, realloc and add it
        int found = 0;
        for (int j = 0; j < nb_prod; j++)
        {
            if (strcmp(products[j].nom, argv[i]) == 0)
            {
                found = 1;
                products[j].quantite += atoi(argv[i + 1]);
                break;
            }
        }
        if (!found)
        {
            products = realloc(products, (nb_prod + 1) * sizeof(struct produit));
            strcpy(products[nb_prod].nom, argv[i]);
            products[nb_prod].quantite = atoi(argv[i + 1]);
            nb_prod++;
        }
    }
    return nb_prod;
}

int file_exists(char *file)
{
    struct stat buffer;
    return (stat(file, &buffer) == 0);
}
