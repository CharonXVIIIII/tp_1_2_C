        define SYSC_EXIT          100
        define SYSC_PUTI          200
        define SYSC_NEW_THREAD    300
        define SYSC_SLEEP         400
        define SYSC_GETCHAR       500

loop:   sysc SYSC_GETCHAR       // AC = getchar()
        sysc SYSC_PUTI          // puti(AC)
        set 1                   // AC = 1
        sysc SYSC_SLEEP         // sleep(AC)
        jump loop

