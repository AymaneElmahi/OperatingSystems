// Fichier client.c à rédiger

//

#include "lib.h"

int main(int argc, char *argv[])
{
    // verifier que le nombre d'arguments est correct
    if (argc < 3 || argc % 2 != 1)
    {
        printf("Usage: %s produit1 quantité1 ... produitn quantitén\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // vérifier que les quantités sont des entiers positifs
    for (int i = 2; i < argc; i += 2)
    {
        int quantite = atoi(argv[i]);
        if (quantite <= 0)
        {
            printf("La quantité doit être un entier positif\n");
            exit(EXIT_FAILURE);
        }
    }

    struct produit *produits = malloc(sizeof(struct produit));
    if (produits == NULL)
    {
        raler(1, "malloc");
    }
    strcpy(produits[0].nom, argv[1]);
    produits[0].quantite = atoi(argv[2]);
    int nb_prod = group_products(produits, argv, argc);

    for (int i = 0; i < nb_prod; i++)
    {
        sem_t *sem_file;
        sem_t *sem_prd;
        set_sem_file(&sem_file, produits[i].nom, 0);
        set_sem(&sem_prd, produits[i].nom, 0);
        TCHK(sem_wait(sem_file));

        int fd;
        CHK(fd = open(produits[i].nom, O_RDWR | O_CREAT, 0666));

        if (is_empty(fd))
        {
            // allow other threads to access the file
            TCHK(sem_post(sem_file));
            CHK(close(fd));
            TCHK(sem_close(sem_prd));
            TCHK(sem_close(sem_file));
            return 0;
        }

        struct produit produit;
        CHK(read(fd, &produit, sizeof(struct produit)));

        int waited = 0;
        while (file_exists(produits[i].nom) && produit.quantite < produits[i].quantite)
        {
            waited = 1;
            // allow other threads to access the file
            TCHK(sem_post(sem_file));
            // say in the file that we are waiting for the product
            produit.clients_waiting++;
            CHK(lseek(fd, 0, SEEK_SET));
            CHK(write(fd, &produit, sizeof(struct produit)));
            // wait for the product
            TCHK(sem_wait(sem_prd));
            CHK(lseek(fd, 0, SEEK_SET));
            CHK(read(fd, &produit, sizeof(struct produit)));
        }

        if (waited)
        {
            TCHK(sem_wait(sem_file));

            if (!file_exists(produits[i].nom))
            {
                produit.clients_waiting--;
                CHK(lseek(fd, 0, SEEK_SET));
                CHK(write(fd, &produit, sizeof(struct produit)));
                TCHK(sem_post(sem_file));
                TCHK(sem_close(sem_prd));
                TCHK(sem_close(sem_file));
                return 1;
            }
            produit.clients_waiting--;
        }

        produit.quantite -= produits[i].quantite;
        CHK(lseek(fd, 0, SEEK_SET));
        CHK(write(fd, &produit, sizeof(struct produit)));
        TCHK(sem_post(sem_file));
        TCHK(sem_close(sem_prd));
        TCHK(sem_close(sem_file));
    }

    return 0;
}