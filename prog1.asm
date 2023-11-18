
//
// Un exemple de boucle avec
// évolution de AC par incrément de 200.
//


    set 3000                    // AC = 3000
loop:                           // définir loop
    iflt end                    // si (AC < 0) aller à end
    sub incr                    // AC = AC - mem[ incr ]
    nop                         // ne rien faire
    jump loop                   // aller à loop
end:
    jump end                    // boucle infinie

incr: data 200                  // valeur de l'increment

