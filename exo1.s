
calcul :                
// * x = %sp+1, y = %sp+2 *
    ld [%sp+1],%a       // a <-- x
    push %b             
// * x = %sp+2, y = %sp+3 *
    ld [%sp+3],%b       // b <-- y
    mul 2,%b            // b <-- 2y
    sub %a,%b           // b <-- 2y-x
    mul -1,%b           // b <-- x-2y
    mul %b,%a           // a <-- x²-2xy

    ld [%sp+3],%b       // b <-- y
    mul %b,%b           // b <--y²

    sub %b,%a           // a <-- x²-2xy-y²
    pop %b              // restore B

    rtn

      
prodscal :              
//* v1 = %sp+1, v2 = %sp+2, n = %sp+3*
    push %b             // v1 = %sp+2, v2 = %sp+3, n = %sp+4
    ld 0,%b             // b <-- i = 0
    push 0              // %sp=0, v1 = %sp+3, v2 = %sp+4, n = %sp+5
    push 1              //%sp=1, %sp+1=0, v1 = %sp+4, v2 = %sp+5, n = %sp+6
    jmp check_i

loop:   ld [%sp+4],%a      // a <--v1
        add %b,%a          // a <--v1[i] (i) (adress)
        ld [%a],%a         // a<-- v1[i]
        st %a,[%sp]        // %sp=v1[i] 
        ld [%sp+5],%a      // a <--v2
        add %b,%a          // a <--v2[i] (i) (adress)
        ld [%a],%a         // a<-- v2[i]
        mul [%sp],%a       // a <-- v1[i]*v2[i]
        add [%sp+1],%a     // a <-- v1[i]*v2[i] +p
        st %a,[%sp+1]      // p=p+v1[i]*v2[i] (in %sp+1)
        add 1,%b           // b <--i+1

check_i : cmp [%sp+6],%b   // compare i and n
          jgt loop         // if i<n go to loop
          ld [%sp+1],%a    // a <-- result
          add 2,%sp        // restore SP
          pop %b           // restore B
          rtn
    


racine : 
    push %b         // %sp+2 = n (b = r)
    push 1          // %sp = 1 = inf, %sp+3 = n
    ld [%sp+3],%a   // a <--sup = n
    push %a         // %sp+4 = n, %sp = n = sup, %sp+1= inf =1
    ld [%sp],%b     // b = r = sup
    sub [%sp+1],%b  // b = sup - inf
    div 2,%b        // b = (sup - inf)/2
    add [%sp+1],%b  // b = r = inf +(sup-inf)/2
    jmp check_while

b2: ld %b,%a        // a <-- r
    mul %a,%a       // a<--r²
    cmp [%sp+4],%a  //compare r² and n
    jlt b3          // if r²>n go to b3
//the "else" case
    st %b,[%sp+1]   // inf = r
    jmp b1          // go to b1

// if(r²>n)
b3: st %b,[%sp]     // sup = r

 // r = inf +(sup-inf)/2
b1: ld [%sp],%b     // b = r = sup
    sub [%sp+1],%b  // b = sup - inf
    div 2,%b        // b = (sup - inf)/2
    add [%sp+1],%b  // b = r = inf +(sup-inf)/2

//check the while statement
check_while: ld %b,%a        // a <-- r
             mul %a,%a       // a<--r²
             cmp [%sp+4],%a  // compare r² and n
             jlt b2          // if r²>n go to b2
             ld %b,%a        // a <-- r
             add 1,%a        // a <-- r+1
             mul %a,%a       // a <-- (r+1)²
             cmp [%sp+4],%a  //compare (r+1)² and n
             jge b2          // if (r+1)²<=n go to b2
             ld %b,%a        // a <-- result
             add 2,%sp       // restore SP
             pop %b          //restore B
             rtn


main_calcul: 
    reset

main_prodscal: 
    reset 

main_racine : 
    reset 