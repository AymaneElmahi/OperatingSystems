
calcul : 
    ld [%sp+1],%a  //a <-- x
    push %b        // on va utiliser b, on le sauvegarde
    ld [%sp+3],%b  // b <-- y
    mul 2,%b       // b <-- 2y
    sub %a,%b      // b <-- 2y-x
    mul -1,%b      // b <-- x-2y
    mul %b,%a      // a <-- x²-2xy
    ld [%sp+3],%b  // b <-- y
    mul %b,%b      // b <--y²
    sub %b,%a      // a <-- x²-2xy-y²
    pop %b         // vider b
    rtn

main_calcul: 
    reset
         
         



prodscal : 

     rtn


racine : 
    rtn





main_prodscal: 
    reset 

main_racine : 
    reset 