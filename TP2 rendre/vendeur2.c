// Fichier vendeur.c à rédiger

#include "lib.h"

// ./vendeur produit quantité, lire le fichier et mettre à jour la quantité
int main(int argc, char *argv[])
{
    // verifier que le nombre d'arguments est correct
    if (argc != 3)
    {
        printf("Usage: %s produit quantité\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // verifier que la quantité est un entier positif
    int quantite = atoi(argv[2]);

    // use named semaphore to synchronize threads with set_sem function
    sem_t *sem_file;
    sem_t *sem_prd;
    set_sem_file(&sem_file, argv[1], 1);
    // set sem_prd as the name of the file with sem_set
    set_sem(&sem_prd, argv[1], 0);
    switch (quantite)
    {
    case 0:
        // verifier que le fichier produit existe, si oui, supprimer le fichier
        if (access(argv[1], F_OK) == 0)
        {
            // read the file
            struct produit produit;
            int fd = open(argv[1], O_RDONLY);
            CHK(read(fd, &produit, sizeof(struct produit)));
            CHK(remove(argv[1]));
            sem_post(sem_file);
            // read file
            for (int i = 0; i <= produit.clients_waiting; i++)
            {
                sem_post(sem_prd);
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Le produit %s n'existe pas\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        return 0;
    default:
        if (quantite < 0)
        {
            printf("La quantité doit être un entier positif\n");
            exit(EXIT_FAILURE);
        }
    }

    // get sem value
    // int sem_value;
    // sem_getvalue(sem_file, &sem_value);
    // printf("sem_file value: %d\n", sem_value);
    // sem_getvalue(sem_prd, &sem_value);
    // printf("sem_prd value: %d\n", sem_value);
    TCHK(sem_wait(sem_file));
    int sem_value;
    sem_getvalue(sem_prd, &sem_value);
    if (sem_value > 0)
        sem_wait(sem_prd);

    int fd;
    CHK(fd = open(argv[1], O_RDWR | O_CREAT, 0666));

    if (is_empty(fd))
    {
        struct produit p;
        p.quantite = quantite;
        strcpy(p.nom, argv[1]);
        p.clients_waiting = 0;
        CHK(write(fd, &p, sizeof(p)));
        // print the new quantity on the file
        // printf("%d", p.quantite);
    }
    else
    {
        struct produit p;
        CHK(read(fd, &p, sizeof(p)));
        p.quantite += quantite;
        CHK(lseek(fd, 0, SEEK_SET));
        CHK(write(fd, &p, sizeof(p)));
        // print the new quantity on the file
        printf("%d\n", p.quantite);
    }

    TCHK(sem_post(sem_file));
    // TCHK(sem_post(sem_file));
    struct produit p;
    CHK(lseek(fd, 0, SEEK_SET));
    CHK(read(fd, &p, sizeof(p)));
    for (int i = 0; i <= p.clients_waiting; i++)
        TCHK(sem_post(sem_prd));

    CHK(close(fd));

    // get sem value
    sem_getvalue(sem_file, &sem_value);
    printf("sem_file value: %d\n", sem_value);
    sem_getvalue(sem_prd, &sem_value);
    printf("sem_prd value: %d\n", sem_value);

    CHK(sem_close(sem_file));
    CHK(sem_close(sem_prd));
    return 0;
}
