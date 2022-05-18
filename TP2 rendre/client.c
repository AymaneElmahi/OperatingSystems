// Fichier client.c à rédiger

#include "lib.h"

// ./client prod1 quant1 prod2 quant2 prod1 quant1*
// make it : prod1 (quant1+quant1*) prod2 quant2 and put it in products
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
    strcpy(produits[0].nom, argv[1]);
    produits[0].quantite = atoi(argv[2]);
    int nb_prod = group_products(produits, argv, argc);

    for (int i = 0; i < nb_prod; i++)
    {
        sem_t *sem_file;
        sem_t *sem_prd;
        sem_file = sem_open("sem_file", O_CREAT, 0666, 1);
        // set sem_prd as the name of the file with sem_set
        sem_prd = set_sem(produits[i].nom, 0);

        // // get sem value
        // int sem_value;
        // sem_getvalue(sem_file, &sem_value);
        // printf("sem_file value: %d\n", sem_value);
        // sem_getvalue(sem_prd, &sem_value);
        // printf("sem_prd value: %d\n", sem_value);

        // TCHK(sem_wait(sem_file));
        TCHK(sem_wait(sem_prd));
        int fd;
        CHK(fd = open(produits[i].nom, O_RDWR | O_CREAT, 0666));
        // if file is empty, exit
        if (is_empty(fd))
        {
            printf("Le produit %s n'existe pas\n", produits[i].nom);
            TCHK(sem_post(sem_prd));
            TCHK(sem_post(sem_file));
            CHK(close(fd));
            TCHK(sem_close(sem_file));
            TCHK(sem_close(sem_prd));

            exit(EXIT_FAILURE);
        }
        // else, buy the product, if not enough, wait till it is
        else
        {
            struct produit p;
            CHK(read(fd, &p, sizeof(p)));
            while (p.quantite < produits[i].quantite)
            {
                TCHK(sem_wait(sem_prd));
            }

            TCHK(sem_wait(sem_file));
            p.quantite -= produits[i].quantite;
            CHK(lseek(fd, 0, SEEK_SET));
            CHK(write(fd, &p, sizeof(p)));
        }
        TCHK(sem_post(sem_file));
        if (produits[i].quantite > 0)
        {
            TCHK(sem_post(sem_prd));
        }
        CHK(close(fd));
        CHK(sem_close(sem_file));
        CHK(sem_close(sem_prd));
    }
    return EXIT_SUCCESS;
}
