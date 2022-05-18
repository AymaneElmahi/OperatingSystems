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
    set_sem(&sem_prd, argv[1], 0);

    switch (quantite)
    {
    case 0:
        // verifier que le fichier produit existe, si oui, supprimer le fichier
        if (access(argv[1], F_OK) == 0)
        {
            // read the file
            struct produit produit;
            int fd;
            CHK(fd = open(argv[1], O_RDONLY));
            CHK(read(fd, &produit, sizeof(struct produit)));
            // delete the file
            CHK(close(fd));
            CHK(unlink(argv[1]));
            for (int i = 0; i <= produit.clients_waiting; i++)
            {
                TCHK(sem_post(sem_prd));
            }
            TCHK(sem_post(sem_file));
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

    TCHK(sem_wait(sem_file));

    int fd = open(argv[1], O_RDWR | O_CREAT, 0666);
    struct produit produit;

    // if the file is empty
    if (is_empty(fd))
    {
        struct produit produit;
        produit.quantite = quantite;
        strcpy(produit.nom, argv[1]);
        produit.clients_waiting = 0;
        CHK(write(fd, &produit, sizeof(struct produit)));
    }
    else
    {
        CHK(read(fd, &produit, sizeof(struct produit)));
        produit.quantite += quantite;
        CHK(ftruncate(fd, 0));
        CHK(lseek(fd, 0, SEEK_SET));
        CHK(write(fd, &produit, sizeof(struct produit)));
        for (int i = 0; i <= produit.clients_waiting; i++)
        {
            TCHK(sem_post(sem_prd));
        }
    }

    TCHK(sem_post(sem_file));
    TCHK(sem_close(sem_prd));
    TCHK(sem_close(sem_file));
    return 0;
}
