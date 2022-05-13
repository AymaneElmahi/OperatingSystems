#!/bin/sh

#
# Script Shell de test de l'exercice 2
# Utilisation : sh ./test2.sh
#
# Si tout se passe bien, le script doit afficher "Tests ok" à la fin
# Dans le cas contraire, le nom du test échoué s'affiche.
# Les fichiers sont laissés dans /tmp/test2*, vous pouvez les examiner
# Pour avoir plus de détails sur l'exécution du script, vous pouvez
# utiliser :
#       sh -x ./test2.sh
# Toutes les commandes exécutées par le script sont alors affichées.
#
# Attention : tous les fichiers commençant par PROD* sont détruits
# par ce script de test.
#

TMP=${TMP:=/tmp/test2}

# il ne faudrait jamais appeler cette fonction
# argument : message d'erreur
fail ()
{
    local msg="$1"

    echo "PAS OK"
    echo "Échec : $msg"
    exit 1
}


# Teste si le processus existe toujours, sinon signale l'erreur
ps_existe ()
{
    [ $# != 2 ] && echo "ERREUR SYNTAXE ps_existe"
    local pid="$1" msg="$2"
    kill -0 $pid 2> /dev/null || fail "$msg"
}

# Renvoie le nombre de processus toujours vivants dans la liste fournie
ps_teste_liste ()
{
    local n=0 pid
    for pid
    do
        if kill -0 $pid 2> /dev/null
        then n=$((n+1))
        fi
    done
    echo $n
}

# Teste si le processus est bien terminé, sinon signale l'erreur
ps_termine ()
{
    [ $# != 2 ] && echo "ERREUR SYNTAXE ps_termine"
    local pid="$1" msg="$2"
    kill -0 $pid 2> /dev/null && fail "$msg"
}

# Conversion ms -> s
# Pratique pour tout gérer en ms dans les scripts de test
mstos ()
{
    [ $# != 1 ] && echo "ERREUR SYNTAXE mstos"
    local ms="$1"
    echo "scale=5;$ms/1000" | bc
}

# Sleep en millisecondes. On suppose que la commande "sleep" accepte
# des nombres flottants, ce qui n'est pas POSIX (argh...) mais qui
# est vrai sur beaucoup de systèmes.
msleep ()
{
    [ $# != 1 ] && echo "ERREUR SYNTAXE msleep"
    local ms="$1"
    sleep $(mstos $ms)
}

# Supprimer tout ce qui peut l'être
raz ()
{
    # Il peut y avoir encore des clients et des vendeurs résultant d'une
    # précédente exécution
    killall client   2> /dev/null
    killall vendeur  2> /dev/null

    # Supprimer les fichiers restant d'une précédente exécution
    rm -f $TMP*
    rm -f PROD*                 # Les fichiers PROD* sont utilisés par ce script
}

##############################################################################

echo -n "Test 1.1 - Syntaxe de vendeur......................................"
raz
./vendeur                       2> $TMP.verr && fail "pas d'argument"
./vendeur PROD1                 2> $TMP.verr && fail "1 argument"
[ -f PROD1 ]                                 && fail "PROD1 créé"
./vendeur PROD1 5 toto          2> $TMP.verr && fail "3 arguments"
[ -f PROD1 ]                                 && fail "PROD1 créé"
./vendeur PROD1 -1              2> $TMP.verr && fail "quantite < 0"
[ -f PROD1 ]                                 && fail "PROD1 créé"
echo OK

##############################################################################

echo -n "Test 1.2 - Syntaxe de client......................................."
raz
./client PROD1                  2> $TMP.cerr && fail "1 argument"
./client PROD1 5 toto           2> $TMP.cerr && fail "3 arguments"
./client PROD1 0 PROD2 5        2> $TMP.cerr && fail "quantité = 0"
echo OK

##############################################################################

echo -n "Test 1.3 - Tentative de fermer un étal inexistant.................."
raz
./vendeur PROD1 0               2> $TMP.verr && fail "1 argument"
echo OK

##############################################################################

echo -n "Test 2.1 - Fonctionnalité basique.................................."
raz
./vendeur PROD1 2 2> $TMP.verr         || fail "erreur vendeur"
[ -f PROD1 ]                           || fail "PROD1 pas créé"

./client PROD1 1 > $TMP.cout1 2> $TMP.cerr1 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID "Le premier client devrait être terminé"
wait $PID                              || fail "erreur client, cf $TMP.cerr1"

./client PROD1 1 > $TMP.cout2 2> $TMP.cerr2 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID "Le deuxième client devrait être terminé"
wait $PID                              || fail "erreur client, cf $TMP.cerr2"

./client PROD1 1 > $TMP.cout3 2> $TMP.cerr3 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 300                                      # attendre un peu plus
ps_existe $PID "Le troisième client devrait être en train d'attendre"

# pour l'instant, on ne s'intéresse pas encore à la terminaison
# et on s'arrête comme des malpropres

echo OK

##############################################################################

echo -n "Test 2.2 - Fonctionnalité basique avec ajout de produits..........."
raz
./vendeur PROD1 2 2> $TMP.verr1        || fail "erreur vendeur 1"
[ -f PROD1 ]                           || fail "PROD1 pas créé"

./client PROD1 1 > $TMP.cout1 2> $TMP.cerr1 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID "Le premier client devrait être terminé"
wait $PID                              || fail "erreur client 1, cf $TMP.cerr1"

./client PROD1 1 > $TMP.cout2 2> $TMP.cerr2 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID "Le deuxième client devrait être terminé"
wait $PID                              || fail "erreur client 2, cf $TMP.cerr2"

./client PROD1 1 > $TMP.cout3 2> $TMP.cerr3 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 300                                      # attendre un peu plus
ps_existe $PID "Le troisième client devrait être en train d'attendre"

./vendeur PROD1 1 2> $TMP.verr2        || fail "erreur vendeur 2"
msleep 100

ps_termine $PID "Le troisième client devrait être terminé"
wait $PID                              || fail "erreur client 3, cf $TMP.cerr3"

# pour l'instant, on ne s'intéresse pas encore à la terminaison
# et on s'arrête comme des malpropres

echo OK

##############################################################################

echo -n "Test 2.3 - Fonctionnalité basique avec plusieurs vendeurs.........."
raz
# démarrer 3 vendeurs
./vendeur PROD1 2 2> $TMP.verr1        || fail "erreur vendeur 1 PROD1 2"
[ -f PROD1 ]                           || fail "PROD1 pas créé"
./vendeur PROD2 5 2> $TMP.verr2        || fail "erreur vendeur 2 PROD2 5"
[ -f PROD2 ]                           || fail "PROD2 pas créé"
./vendeur PROD3 2 2> $TMP.verr3        || fail "erreur vendeur 1 PROD3 2"
[ -f PROD3 ]                           || fail "PROD3 pas créé"
# stock = <2, 5, 2>, attente = <>

./client PROD1 1 PROD2 2 PROD3 2 > $TMP.cout1 2> $TMP.cerr1 &
PID1=$!                                         # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID1 "Le premier client devrait être terminé"
wait $PID1                            || fail "erreur client 1, cf $TMP.cerr1"
# stock = <1, 3, 0>, attente = <>

./client PROD2 1 PROD3 2         > $TMP.cout2 2> $TMP.cerr2 &
PID2=$!
msleep 100
ps_existe $PID2 "Le deuxième client devrait attendre (plus de PROD3)"
# stock = <1, 2, 0>, attente = <c2>

./client PROD1 1 PROD2 1         > $TMP.cout3 2> $TMP.cerr3 &
PID3=$!
msleep 100
ps_termine $PID3 "Le troisième client devrait être terminé"
wait $PID3                            || fail "erreur client 3, cf $TMP.cerr3"
# stock = <0, 1, 0>, attente = <c2>

# réapprovisionner en PROD3
./vendeur PROD3 5 > $TMP.verr4        || fail "erreur vendeur 4 PROD3 5"
# stock = <0, 1, 5>, attente = <c2>
# laisser le temps au client 2 de se réveiller
msleep 100
ps_termine $PID2 "Le deuxième client a eu son PROD3, donc devrait être terminé"
wait $PID2                            || fail "erreur client 2, cf $TMP.cerr2"
# stock = <0, 1, 3>, attente = <>

# juste pour vérifier que tout continue d'aller bien
./client PROD2 1 > $TMP.cout4 2> $TMP.cerr4 &
PID4=$!                                         # noter son pid
msleep 100                                      # attendre un peu plus
ps_termine $PID4 "Le quatrième client devrait être terminé"
wait $PID4                            || fail "erreur client 4, cf $TMP.cerr4"
# stock = <0, 0, 3>, attente = <>

# pour l'instant, on ne s'intéresse pas encore à la terminaison
# et on s'arrête comme des malpropres

echo OK

##############################################################################

echo -n "Test 3.1 - Départ du vendeur avec un seul client..................."
raz
./vendeur PROD1 2 2> $TMP.verr1       || fail "erreur vendeur"
[ -f PROD1 ]                          || fail "PROD1 pas créé"

./client PROD1 1 > $TMP.cout1 2> $TMP.cerr1 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID "Le premier client devrait être terminé"
wait $PID                             || fail "erreur client, cf $TMP.err1"

./client PROD1 1 > $TMP.cout2 2> $TMP.cerr2 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID "Le deuxième client devrait être terminé"
wait $PID                             || fail "erreur client, cf $TMP.err2"

./client PROD1 1 > $TMP.cout3 2> $TMP.cerr3 &   # lancer client en arrière plan
PID=$!                                          # noter son pid
msleep 300                                      # attendre un peu plus
ps_existe $PID "Le troisième client devrait être en train d'attendre"

# puis le vendeur ferme
./vendeur PROD1 0 2> $TMP.verr2       || fail "erreur fermeture vendeur"
[ -f PROD1 ]                          && fail "PROD1 ne devrait plus être là"

msleep 100
ps_termine $PID "Le troisième client devrait être rentré chez lui"
wait $PID                             && fail "le 3e client devrait exit(1)"

echo OK

##############################################################################

echo -n "Test 3.2 - Départ du vendeur avec clients multiples................"
raz
N=100
./vendeur PROD1 1 2> $TMP.verr1       || fail "erreur vendeur"
[ -f PROD1 ]                          || fail "PROD1 pas créé"

./client PROD1 1 > $TMP.cout0 2> $TMP.cerr0 &
PID=$!                                          # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID "Le client initial devrait être terminé"
wait $PID                             || fail "erreur client init, cf $TMP.err0"

LPID=""
for i in $(seq 1 $N)
do
    ./client PROD1 1 > $TMP.cout$i 2> $TMP.cerr$i &
    LPID="$LPID $!"
done
msleep 100
n=$(ps_teste_liste $LPID)
[ $n != $N ]                          && fail "$N clients devraient attendre ($n)"

# on ferme !
./vendeur PROD1 0 2> $TMP.verr2       || fail "erreur départ vendeur"

msleep 100
n=$(ps_teste_liste $LPID)
[ $n != 0 ]                           && fail "0 clients devraient attendre ($n)"

i=1
for pid in $LPID
do
    wait $pid                         && fail "le client $i devrait exit(1)"
    i=$((i+1))
done

echo OK

##############################################################################

echo -n "Test 3.3 - Départ du vendeur et arrêt des courses.................."
raz
./vendeur PROD1 1 2> $TMP.verr1       || fail "erreur vendeur 1"
[ -f PROD1 ]                          || fail "PROD1 pas créé"
./vendeur PROD2 2 2> $TMP.verr2       || fail "erreur vendeur 2"
[ -f PROD2 ]                          || fail "PROD2 pas créé"
# stock = <1, 2>, attente = <>

./client PROD1 1 PROD2 1 > $TMP.cout1 2> $TMP.cerr1 &
PID1=$!
msleep 100
ps_termine $PID1 "Le premier client devrait être terminé"
wait $PID1                            || fail "erreur client, cf $TMP.err1"
# stock = <0, 1>, attente = <>

./client PROD1 1 PROD2 1 > $TMP.cout2 2> $TMP.cerr2 &
PID2=$!
msleep 100
ps_existe $PID2 "Le deuxième client devrait attendre"
# stock = <0, 2>, attente = <c2>

# Le premier vendeur ferme boutique
./vendeur PROD1 0 2> $TMP.verr3       || fail "erreur départ vendeur 1"
# stock = <-, 2>, attente = <>

# Le deuxième client devrait être terminé
msleep 100
ps_termine $PID2 "Le deuxième client devrait être terminé"
wait $PID2                            && fail "le client 2 devrait exit(1)"
# stock = <-, 1>, attente = <>

# Normalement, le stock de PROD2 devrait contenir encore un article
./client PROD2 1 > $TMP.cout3 2> $TMP.cerr3 &
PID3=$!                                         # noter son pid
msleep 100                                      # attendre 100 ms
ps_termine $PID3 "Le troisième client devrait trouver PROD2"
wait $PID3                            || fail "le client 3 devrait exit(0)"
# stock = <-, 0>, attente = <>

# Le deuxième vendeur ferme lui aussi boutique
./vendeur PROD2 0 2> $TMP.verr4       || fail "erreur départ vendeur 2"

echo OK

##############################################################################

echo -n "Test 4.1 - Un peu de concurrence..................................."
raz
N=1000
./vendeur PROD1 $N 2> $TMP.verr1     || fail "erreur vendeur 1"
[ -f PROD1 ]                         || fail "PROD1 pas créé"
# on lance $N clients
LPID=""
for i in $(seq 1 $N)
do
    ./client PROD1 1 > $TMP.cout$i 2> $TMP.cerr$i &
    LPID="$LPID $!"
done
msleep 100
n=$(ps_teste_liste $LPID)
[ $n = 0 ]                            || fail "aucun client ne devrait attendre"
./client PROD1 1 > $TMP.outn 2> $TMP.errn &
PID=$!
msleep 100
ps_existe $PID "Le client numéro $((N+1)) devrait attendre"

./vendeur PROD1 1 2> $TMP.verr2       || fail "erreur vendeur 2"

msleep 100
ps_termine $PID "Le client numéro $((N+1)) devrait être terminé"
wait $PID                             || fail "client $((N+1)) en erreur"

echo OK

##############################################################################

echo -n "Test 4.2 - Tentative de provoquer une situation d'interblocage....."
raz

./vendeur PROD1 1 2> $TMP.verr       || fail "erreur vendeur PROD1"
[ -f PROD1 ]                         || fail "PROD1 pas créé"
# vider le stock
./client PROD1 1 > $TMP.out0 2> $TMP.err0 || fail "erreur client 0"

# il faut insister un peu pour créer une situation potentielle d'interblocage
N=100
for i in $(seq 1 $N)
do
    ./client PROD1 1 > $TMP.cout$i 2> $TMP.cerr$i &
    PID1=$!
    ./vendeur PROD1 1 > $TMP.vout$i 2> $TMP.verr$i &
    PID2=$!

    msleep 50
    ps_termine $PID1 "Itération $i : client non terminé"
    ps_termine $PID2 "Itération $i : vendeur non terminé"

    wait $PID1                       || fail "erreur client itération $i"
    wait $PID2                       || fail "erreur vendeur itération $i"
done

./vendeur PROD1 0 2> $TMP.verr-fin  || fail "erreur départ vendeur"

echo OK

raz
exit 0
