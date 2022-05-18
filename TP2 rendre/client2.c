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

int file_exists(char *file)
{
    struct stat buffer;
    return (stat(file, &buffer) == 0);
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
        set_sem_file(&sem_file, produits[i].nom, 0);
        set_sem(&sem_prd, produits[i].nom, 0);
        printf("%s\n", produits[i].nom);

        // get sem value
        int sem_value;
        sem_getvalue(sem_file, &sem_value);
        printf("sem_file value: %d\n", sem_value);
        sem_getvalue(sem_prd, &sem_value);
        printf("sem_prd value: %d\n", sem_value);

        TCHK(sem_wait(sem_file));
        // TCHK(sem_wait(sem_prd));
        int fd;
        CHK(fd = open(produits[i].nom, O_RDWR | O_CREAT, 0666));
        // if file is empty, exit
        if (is_empty(fd))
        {
            printf("Le produit %s n'existe pas\n", produits[i].nom);
            // TCHK(sem_post(sem_prd));
            TCHK(sem_post(sem_file));
            CHK(close(fd));
            TCHK(sem_close(sem_file));
            TCHK(sem_close(sem_prd));

            return EXIT_FAILURE;
        }
        // else, buy the product, if not enough, wait till it is
        else
        {
            struct produit p;
            int we_waited = 0;
            CHK(read(fd, &p, sizeof(p)));
            int sem_value;
            sem_getvalue(sem_prd, &sem_value);
            printf("sem_prd before value: %d\n", sem_value);
            while (p.quantite < produits[i].quantite && file_exists(produits[i].nom))
            {
                printf("Le produit %s n'est pas disponible\n", produits[i].nom);
                TCHK(sem_post(sem_file));
                we_waited = 1;

                CHK(lseek(fd, 0, SEEK_SET));
                CHK(read(fd, &p, sizeof(p)));
                p.clients_waiting++;
                CHK(write(fd, &p, sizeof(p)));

                TCHK(sem_wait(sem_prd));
                TCHK(sem_wait(sem_prd));
                CHK(lseek(fd, 0, SEEK_SET));
                CHK(read(fd, &p, sizeof(p)));
            }
            // if we waited, we have to wait for the other clients to finish
            if (!file_exists(produits[i].nom))
            {
                printf("Le produit %s n'existe pas\n", produits[i].nom);
                // TCHK(sem_post(sem_prd));
                TCHK(sem_post(sem_file));
                CHK(close(fd));
                TCHK(sem_close(sem_file));
                TCHK(sem_close(sem_prd));

                return EXIT_FAILURE;
            }
            if (we_waited)
                TCHK(sem_wait(sem_file));

            p.quantite -= produits[i].quantite;
            CHK(lseek(fd, 0, SEEK_SET));
            CHK(write(fd, &p, sizeof(p)));
        }
        TCHK(sem_post(sem_file));
        // TCHK(sem_post(sem_prd));
        // if (produits[i].quantite > 0)
        // {
        //     TCHK(sem_post(sem_prd));
        // }
        CHK(close(fd));
        CHK(sem_close(sem_file));
        CHK(sem_close(sem_prd));
    }
    return EXIT_SUCCESS;
}
