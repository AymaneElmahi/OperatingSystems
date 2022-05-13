#
# Attention : vous ne devez pas modifier ce fichier.
#
# Ce Makefile contient les cibles suivantes :
#
# all		: compile les programmes
# clean		: supprime les fichiers générés automatiquement
# test		: lance les tests (scripts shell test2.sh)
# devoir.tgz	: construit le fichier devoir.tgz que vous devez déposer
#		  sur Moodle
#

CFLAGS = -g -Wall -Wextra -Werror
LDLIBS = -g -lpthread -lrt

# Les fichiers attendus pour le rendu sur le devoir Moodle
DEVOIR = lib.c lib.h client.c vendeur.c

PROGS = client vendeur

all: $(PROGS)

#
# Vous devez créer un fichier lib.c contenant les fonctions communes
# aux deux programmes et un fichier lib.h pour vos définitions.
#

$(PROGS): lib.o

lib.o: lib.h
client.o: lib.h
vendeur.o: lib.h

test: $(PROGS)
	sh timeout.sh test2.sh || exit 1

# génère un fichier devoir.tgz contenant les fichiers à déposer sur Moodle
devoir.tgz: $(DEVOIR)
	tar czvf devoir.tgz $(DEVOIR)

clean:
	rm -f *.o $(PROGS)
	rm -f PROD*			# fichiers utilisés pour les tests
	rm -f tags core moodle.tgz
	rm -rf moodle
	rm -f devoir.tgz

