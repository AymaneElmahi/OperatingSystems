#!/bin/sh

PROG=${PROG:=exo1.s}		# nom de votre fichier assembleur Zorglub33
Z33=${Z33CLI:=./z33-cli}	# chemin de l'exécutable z33-cli

#
# Script Shell de test de l'exercice 1
# Utilisation : sh ./test1.sh
#
# Si tout se passe bien, le script doit afficher "Tests ok" à la fin
# Dans le cas contraire, le nom du test échoué s'affiche.
# Les fichiers sont laissés dans /tmp/test2*, vous pouvez les examiner
# Pour avoir plus de détails sur l'exécution du script, vous pouvez
# utiliser :
#	sh -x ./test1.sh
# Toutes les commandes exécutées par le script sont alors affichées.
#

TMP=/tmp/test1

# il ne faudrait jamais appeler cette fonction
# argument : message d'erreur
fail ()
{
    local msg="$1"

    echo "PAS OK"
    echo "Échec : $msg"
    exit 1
}

# vérifie que la version de z33-cli est bien >= 0.5.0
# $1 : exécutable z33-cli
tester_version ()
{
    local z33="$1"
    local v1 v2 v3
    local vmin="0 5 0" m1 m2 m3

    set $vmin
    m1=$1 ; m2=$2 ; m3=$3
    set $("$z33" -V | sed -e 's/.* //' -e 's/\./ /g')
    v1=$1 ; v2=$2 ; v3=$3
    if [ "$v1" -lt $m1 ]
    then fail "Version $z33 = $v1.*.* insuffisante, min=$m1.$m2.$m3"
    elif [ "$v2" -lt $m2 ]
    then fail "Version $z33 = $v1.$v2.* insuffisante, min=$m1.$m2.$m3"
    elif [ "$v3" -lt $m3 ]
    then fail "Version $z33 = $v1.$v2.$v3 insuffisante, min=$m1.$m2.$m3"
    fi
    set +x
}

# cherche le label dans le fichier
# $1 = label à chercher
chercher_label ()
{
    [ $# != 1 ] && fail "ERREUR SYNTAXE chercher_label"
    local label="$1"
    grep -q "^$label[ 	]*:" $PROG || fail "Label '$label' non trouvé"
}

# cherche un reset dans le programme assembleur
chercher_reset ()
{
    [ $# != 0 ] && fail "ERREUR SYNTAXE chercher_reset"
}

# lance l'émulateur sur le point d'entrée
# $1 = instructions
tester ()
{
    [ $# = 0 ] && fail "ERREUR SYNTAXE tester"
    (
	cat "$PROG"
	echo
	echo "test:"
	for i
	do
	    echo "	$i"
	done
    ) > $TMP.in 
    $Z33 run $TMP.in test > $TMP.1 2> $TMP.2 \
	|| fail "erreur sur '$label' => regarder $TMP.in, $TMP.1 et $TMP.2"
}

# vérifier le résultat de l'exécution
# $1 = registre
# $2 = valeur
# $3 = msg d'erreur
verifier ()
{
    [ $# != 3 ] && fail "ERREUR SYNTAXE verifier"
    local reg="$1" val="$2" msg="$3"
    local v

    v=$(sed -n "/End of program/s/.*%$reg = \([^ ]*\) .*/\1/p" $TMP.1)
    [ x"$v" = x"$val" ] || fail "$msg
	Valeur trouvée = $v (attendue = $val) pour registre %$reg
	=> regarder $TMP.in et $TMP.1"
}


# Supprimer les fichiers restant d'une précédente exécution
rm -rf $TMP*

# Recherche des labels demandés dans l'énoncé
echo -n "Test 1.1 - Fichiers présents..................................."
[ -f "$PROG" ] || fail "Fichier '$PROG' non trouvé"
[ -x "$Z33" ] || fail "Exécutable '$Z33' non trouvé"
tester_version "$Z33"
echo OK

# Recherche des labels demandés dans l'énoncé
echo -n "Test 1.2 - Labels des fonctions demandées dans l'énoncé........"
chercher_label "calcul"
chercher_label "prodscal"
chercher_label "racine"
echo OK

# Recherche des labels "main_*" demandés dans l'énoncé
echo -n "Test 1.3 - Labels des 'main' demandés dans l'énoncé............"
chercher_label "main_calcul"
chercher_label "main_prodscal"
chercher_label "main_racine"
echo OK

# Il doit y avoir un reset pour chaque main_* (pour que les prog. s'arrêtent)
# On se contente de tester la présence d'un seul reset pour simplifier
echo -n "Test 1.4 - Au moins un 'reset'................................."
grep -q "^[ 	]*reset" $PROG || fail "Instruction 'reset' non trouvée"
echo OK

# Tentatives d'exécution des "main_*" pour voir s'ils se terminent
echo -n "Test 2.1 - Test 'main_calcul'.................................."
tester "jmp main_calcul"
echo OK

echo -n "Test 2.2 - Test 'main_prodscal'................................"
tester "jmp main_prodscal"
echo OK

echo -n "Test 2.3 - Test 'main_racine'.................................."
tester "jmp main_racine"
echo OK

# Test de 'calcul'
echo -n "Test 3.1 - Test 'calcul' 1....................................."
tester  "ld 4567,%b" "push 7" "push 4" "call calcul" "reset"
echo OK
echo -n "Test 3.2 - Test 'calcul' 1 - retour dans A....................."
verifier a -89   "calcul incorrect dans A"
echo OK
echo -n "Test 3.3 - Test 'calcul' 1 - B restauré........................"
verifier b 4567  "registre B non restauré"
echo OK
echo -n "Test 3.4 - Test 'calcul' 1 - SP restauré......................."
verifier sp 9998 "registre SP non restauré"
echo OK

echo -n "Test 3.5 - Test 'calcul' 2....................................."
tester  "ld 8002,%sp" "ld 987,%b" "push 10" "push 25" "call calcul" "reset"
echo OK
echo -n "Test 3.6 - Test 'calcul' 2 - retour dans A....................."
verifier a 25   "calcul incorrect dans A"
echo OK
echo -n "Test 3.7 - Test 'calcul' 2 - B restauré........................"
verifier b 987  "registre B non restauré"
echo OK
echo -n "Test 3.8 - Test 'calcul' 2 - SP restauré......................."
verifier sp 8000 "registre SP non restauré"
echo OK

# Test de 'prodscal'
echo -n "Test 4.1 - Test 'prodscal' 1..................................."
tester "ld 4567,%b" "push 2" "push testv2" "push testv1" \
	"call prodscal" "reset" \
	"testv1:" ".word 3" ".word 5" \
	"testv2:" ".word 2" ".word 4"
echo OK
echo -n "Test 4.2 - Test 'prodscal' 1 - retour dans A..................."
verifier a 26    "calcul incorrect dans A"
echo OK
echo -n "Test 4.3 - Test 'prodscal' 1 - B restauré......................"
verifier b 4567  "registre B non restauré"
echo OK
echo -n "Test 4.4 - Test 'prodscal' 1 - SP restauré....................."
verifier sp 9997 "registre SP non restauré"
echo OK

echo -n "Test 4.5 - Test 'prodscal' 2..................................."
tester "ld 8003,%sp" "ld 987,%b" "push 5" "push testv2" "push testv1" \
	"call prodscal" "reset" \
	"testv2:" ".word 4" ".word -7" ".word 7" ".word -12" ".word 8" \
	"testv1:" ".word 1" ".word 18" ".word -2" ".word 9" ".word -6"
echo OK
echo -n "Test 4.6 - Test 'prodscal' 2 - retour dans A..................."
verifier a -292  "calcul incorrect dans A"
echo OK
echo -n "Test 4.7 - Test 'prodscal' 2 - B restauré......................"
verifier b 987   "registre B non restauré"
echo OK
echo -n "Test 4.8 - Test 'prodscal' 2 - SP restauré....................."
verifier sp 8000 "registre SP non restauré"
echo OK

# Test de 'racine'
echo -n "Test 5.1 - Test 'racine' 1....................................."
tester  "ld 4567,%b" "push 25" "call racine" "reset"
echo OK
echo -n "Test 5.2 - Test 'racine' 1 - retour dans A....................."
verifier a 5     "calcul incorrect dans A"
echo OK
echo -n "Test 5.3 - Test 'racine' 1 - B restauré........................"
verifier b 4567  "registre B non restauré"
echo OK
echo -n "Test 5.4 - Test 'racine' 1 - SP restauré......................."
verifier sp 9999 "registre SP non restauré"
echo OK

echo -n "Test 5.5 - Test 'racine' 2....................................."
tester  "ld 8001,%sp" "ld 987,%b" "push 171" "call racine" "reset"
echo OK
echo -n "Test 5.6 - Test 'racine' 2 - retour dans A....................."
verifier a 13   "calcul incorrect dans A"
echo OK
echo -n "Test 5.7 - Test 'racine' 2 - B restauré........................"
verifier b 987  "registre B non restauré"
echo OK
echo -n "Test 5.8 - Test 'racine' 2 - SP restauré......................."
verifier sp 8000 "registre SP non restauré"
echo OK

rm -f $TMP.*
exit 0
