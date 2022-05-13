// Fichier vendeur.c à rédiger

#include "lib.h"

int is_empty(int fd)
{
    struct stat s;
    CHK(fstat(fd, &s));
    return s.st_size == 0;
}

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
    switch (quantite)
    {
    case 0:
        // verifier que le fichier produit existe, si oui, fermer le fichier
        if (access(argv[1], F_OK) == 0)
        {
            int fd = open(argv[1], O_RDWR);
            CHK(fd);
            CHK(close(fd));
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

    int fd;
    CHK(fd = open(argv[1], O_RDWR | O_CREAT, 0666));

    if (is_empty(fd))
    {
        struct produit p;
        p.quantite = quantite;
        CHK(write(fd, &p, sizeof(p)));
    }
    else
    {
        struct produit p;
        CHK(read(fd, &p, sizeof(p)));
        p.quantite += quantite;
        CHK(lseek(fd, 0, SEEK_SET));
        CHK(write(fd, &p, sizeof(p)));
    }
}
