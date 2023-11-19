
//
// Un exemple de boucle avec
// évolution de AC par incrément de 200.
//

 define SYSC_EXIT            100
    define SYSC_PUTI            200
    define SYSC_NEW_THREAD      300

    // *** créer un thread ***
    sysc SYSC_NEW_THREAD        // créer un thread
    ifgt pere                   // si (AC > 0), aller à pere 

    // *** code du fils ***
    set 1000                    // AC = 1000    
    sysc SYSC_PUTI              // afficher AC  
    nop
    nop  

pere: // *** code du père ***
    set 2000                    // AC = 2000      
    sysc SYSC_PUTI              // afficher AC    
    sysc SYSC_EXIT              // fin du thread  