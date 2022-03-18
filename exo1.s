
calcul : 
    ld [%sp+1],%a       //a <-- x
    push %b             // on va utiliser b, on le sauvegarde
    ld [%sp+3],%b       // b <-- y
    mul 2,%b            // b <-- 2y
    sub %a,%b           // b <-- 2y-x
    mul -1,%b           // b <-- x-2y
    mul %b,%a           // a <-- x²-2xy
    ld [%sp+3],%b       // b <-- y
    mul %b,%b           // b <--y²
    sub %b,%a           // a <-- x²-2xy-y²
    pop %b              // vider b
    rtn

      
prodscal :              //v1 = %sp+1, v2 = %sp+2, n = %sp+3
    push %b             //v1 = %sp+2, v2 = %sp+3, n = %sp+4
    ld 0,%b             //b <-- i=0
    push 0             //%sp=0, v1 = %sp+3, v2 = %sp+4, n = %sp+5
    push 1            //%sp=1, %sp+1=0, v1 = %sp+4, v2 = %sp+5, n = %sp+6
    jmp b2
b1 : ld [%sp+4],%a      // a <--v1
     add %b,%a          // a <--v1[i] (i) (adresse)
     ld [%a],%a         // a<-- v1[i]
     st %a,[%sp]        // %sp=v1[i] 
     ld [%sp+5],%a      // a <--v2
     add %b,%a          // a <--v2[i] (i) (adresse)
     ld [%a],%a         // a<-- v2[i]
     mul [%sp],%a       // a <-- v1[i]*v2[i]
     add [%sp+1],%a     // a <-- v1[i]*v2[i] +p
     st %a,[%sp+1]      // p=p+v1[i]*v2[i] (dans %sp+1)
b3 : add 1,%b           // b <--i+1
b2 : cmp [%sp+6],%b     // compare i and n
     jgt b1
     ld [%sp+1],%a
     add 2,%sp
     pop %b
     rtn
    


racine : 
    push %b         // %sp+2 = n (on mettra r ici)
    push 1          // %sp = 1 = inf, %sp+3 = n
    ld [%sp+3],%a   // a <--sup = n
    push %a         // %sp+4 = n, %sp = n = sup, %sp+1= inf =1
    ld [%sp],%b     // b = r = sup
    sub [%sp+1],%b  // b = sup - inf
    div 2,%b        // b = (sup - inf)/2
    add [%sp+1],%b  // b = r = inf +(sup-inf)/2
    jmp c2
c3: ld %b,%a        // a <-- r
    mul %a,%a       // a<--r²
    cmp [%sp+4],%a
    jlt c5          // if r²>n go to c5
    st %b,[%sp+1]   // else inf = r
    jmp c4
c5: st %b,[%sp]     // sup = r
c4: ld [%sp],%b     // b = r = sup
    sub [%sp+1],%b  // b = sup - inf
    div 2,%b        // b = (sup - inf)/2
    add [%sp+1],%b  // b = r = inf +(sup-inf)/2
c2: ld %b,%a        // a <-- r
    mul %a,%a       // a<--r²
    cmp [%sp+4],%a
    jlt c3
    ld %b,%a
    add 1,%a
    mul %a,%a 
    cmp [%sp+4],%a
    jge c3
    ld %b,%a
    add 2,%sp
    pop %b
    rtn


main_calcul: 
    push 1
    push 2
    call calcul
    reset

main_prodscal: 
    push 4
    push 2
    push 3
    call prodscal
    reset 

main_racine : 
    push 4
    call racine
    reset 