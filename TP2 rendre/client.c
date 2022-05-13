// Fichier client.c à rédiger

#include "lib.h"

//  ./client produit1 quantité1 ... produitn quantitén
// Ce programme simule un client avec sa liste de courses : il doit acheter les produits indiqués. Si un
// produit est inconnu, c’est-à-dire si le fichier correspondant est vide, le client râle et se termine (avec
// ou sans les produits déjà achetés, selon ce qui vous semblera le plus simple). Si un produit n’est pas
// disponible, le client attend que le commerçant réapprovisionne son étal. Si le commerçant ferme son
// étal, le client râle et arrête là ses courses
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

    for (int i = 1; i < argc; i += 2)
    {
        // vérifier que le fichier n'est pas vide
        int fd;
        CHK(fd = open(argv[i], O_RDWR | O_CREAT, 0666));
        struct stat s;
        CHK(fstat(fd, &s));
        if (s.st_size == 0)
        {
            printf("Le produit %s n'existe pas\n", argv[i]);
            exit(EXIT_FAILURE);
        }

        // vérifier que le produit est disponible
        struct produit p;
        CHK(read(fd, &p, sizeof(p)));
        if (p.quantite == 0)
        {
            printf("Le produit %s n'est pas disponible\n", argv[i]);
            exit(EXIT_FAILURE);
        }
        else
        {
            // mettre à jour la quantité
            p.quantite -= atoi(argv[i + 1]);
            CHK(lseek(fd, 0, SEEK_SET));
            CHK(write(fd, &p, sizeof(p)));
        }
    }

    return 0;
}
