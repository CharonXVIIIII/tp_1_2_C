
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "cpu.h"


/* ============================================================================
 _   _ _____   ____   _    ____    __  __  ___  ____ ___ _____ ___ _____ ____  
| \ | | ____| |  _ \ / \  / ___|  |  \/  |/ _ \|  _ \_ _|  ___|_ _| ____|  _ \ 
|  \| |  _|   | |_) / _ \ \___ \  | |\/| | | | | | | | || |_   | ||  _| | |_) |
| |\  | |___  |  __/ ___ \ ___) | | |  | | |_| | |_| | ||  _|  | || |___|  _ < 
|_| \_|_____| |_| /_/   \_\____/  |_|  |_|\___/|____/___|_|   |___|_____|_| \_\
                                                                                                                                   
============================================================================ */

/**********************************************************
** définition de la mémoire pysique simulée
***********************************************************/

#define MAX_MEM        (128)

#define IS_PHYSICAL_ADR(a)    ((0 <= (a)) && ((a) < MAX_MEM))
#define IS_LOGICAL_ADR(a,cpu) (IS_PHYSICAL_ADR(a))

static WORD mem[MAX_MEM];     /* mémoire                       */


/**********************************************************
** initialiser la machine (notamment la mémoire)
***********************************************************/

static void init_cpu() {
    static int init_done = 0;
    if (init_done) return;
    
    // vérifier la taille des structures
    assert(sizeof(WORD) == sizeof(INST));
    
    for(int adr=0; (adr < MAX_MEM); adr++) {
        mem[adr] = -1;
    }
    init_done = 1;
}


/**********************************************************
** Lire ou écrire une case de la mémoire physique
***********************************************************/

WORD read_mem(int physical_address) {
    init_cpu();
    if (! IS_PHYSICAL_ADR(physical_address)) {
        fprintf(stderr, "ERROR: read_mem: bad address %d\n", physical_address);
        exit(EXIT_FAILURE);
    }
    return mem[physical_address];
}


void write_mem(int physical_address, WORD value) {
    init_cpu();
    if (! IS_PHYSICAL_ADR(physical_address)) {
        fprintf(stderr, "ERROR: write_mem: bad address %d\n", physical_address);
        exit(EXIT_FAILURE);
    }
    mem[physical_address] = value;
}


/**********************************************************
** Lire une case de la mémoire logique
***********************************************************/

static WORD read_logical_mem(int logical_address, PSW* cpu) {
    if (! IS_LOGICAL_ADR(logical_address, *cpu)) {
        cpu->IN = INT_SEGV;
        return (0);
    }
    return read_mem(logical_address);
}


/**********************************************************
** écrire une case de la mémoire logique
***********************************************************/

static void write_logical_mem(int logical_address, PSW* cpu, WORD value) {
    if (! IS_LOGICAL_ADR(logical_address, *cpu)) {
        cpu->IN = INT_SEGV;
        return;
    }
    write_mem(logical_address, value);
}


/**********************************************************
** Coder une instruction
***********************************************************/

WORD encode_instruction(INST instr) {
    union { WORD word; INST instr; } instr_or_word;
    instr_or_word.instr = instr;
    return (instr_or_word.word);
}


/**********************************************************
** Décoder une instruction
***********************************************************/

INST decode_instruction(WORD value) {
    union { WORD integer; INST instruction; } inst;
    inst.integer = value;
    return inst.instruction;
}


/**********************************************************
** instruction d'addition
**
** ADD arg
**   | AC := (AC + mem[arg])
**   | PC := (PC + 1)
***********************************************************/

static PSW cpu_ADD(PSW m) {
    WORD value = read_logical_mem(m.RI.arg, &m);
    // erreur de lecture en mémoire logique
    if (m.IN) return (m);
    m.AC += value;
    m.PC += 1;
    return m;
}


/**********************************************************
** instruction de soustraction
**
** SUB arg
**   | AC := (AC - mem[arg])
**   | PC := (PC + 1)
***********************************************************/

static PSW cpu_SUB(PSW m) {
    WORD value = read_logical_mem(m.RI.arg, &m);
    // erreur de lecture en mémoire logique
    if (m.IN) return (m);
    m.AC -= value;
    m.PC += 1;
    return m;
}


/**********************************************************
** affectation de l'accumulateur
**
** SET arg
**   | AC := arg
**   | PC := (PC + 1)
***********************************************************/

static PSW cpu_SET(PSW m) {
    m.AC = m.RI.arg;
    m.PC += 1;
    return m;
}


/**********************************************************
** ne rien faire (presque)
**
** NOP
**   | PC := (PC + 1)
***********************************************************/

static PSW cpu_NOP(PSW m) {
    m.PC += 1;
    return m;
}


/**********************************************************
** lire la mémoire dans l'accumulateur
**
** LOAD adr
**   | AC := mem[ adr ]
**   | PC := (PC + 1)
***********************************************************/

static PSW cpu_LOAD(PSW m) {
    WORD address = (m.RI.arg);
    WORD value = read_logical_mem(address, &m);

    // erreur de lecture en mémoire logique
    if (m.IN) return (m);

    // lecture réussie
    m.AC = value;
    m.PC += 1;
    return m;
}


/**********************************************************
** sauver l'accumulateur en mémoire.
**
** STORE adr
**   | mem[ adr ] := AC
**   | PC := (PC + 1)
***********************************************************/

static PSW cpu_STORE(PSW m) {
    WORD address = (m.RI.arg);
    write_logical_mem(address, &m, m.AC);
    
    // erreur d'écriture en mémoire
    if (m.IN) return (m);

    // écriture réussie
    m.PC += 1;
    return m;
}


/**********************************************************
** saut inconditionnel.
**
** JUMP offset
**   | PC := offset
***********************************************************/

static PSW cpu_JUMP(PSW m) {
    m.PC = m.RI.arg;
    return m;
}


/**********************************************************
** saut si plus grand que.
**
** IFGT offset
**   | if (AC > 0) PC := offset
**   | else PC := (PC + 1)
***********************************************************/

static PSW cpu_IFGT(PSW m) {
    if (m.AC > 0) {
        m.PC = m.RI.arg;
    } else {
        m.PC += 1;
    }
    return m;
}


/**********************************************************
** saut si égale.
**
** IFEQ offset
**   | if (AC == 0) PC := offset
**   | else PC := (PC + 1)
***********************************************************/

static PSW cpu_IFEQ(PSW m) {
    if (m.AC == 0) {
        m.PC = m.RI.arg;
    } else {
        m.PC += 1;
    }
    return m;
}


/**********************************************************
** saut si plus petit que.
**
** IFLT offset
**   | if (AC < 0) PC := offset
**   | else PC := (PC + 1)
***********************************************************/

static PSW cpu_IFLT(PSW m) {
    if (m.AC < 0) {
        m.PC = m.RI.arg;
    } else {
        m.PC += 1;
    }
    return m;
}


/**********************************************************
** appel au système (interruption SYSC)
**
** SYSC arg
**   | PC := (PC + 1)
**   | <interruption cause SYSC>
***********************************************************/

static PSW cpu_SYSC(PSW m) {
    m.PC += 1;
    m.IN = INT_SYSC;
    return m;
}


/**********************************************************
** génération d'une interruption clavier toutes les
** trois secondes.
***********************************************************/

static PSW keyboard_event(PSW m) {
    static time_t next_kbd_event = 0;
    static char* data_sample = "Keyboard DATA.\n";
    static int kbd_data_index = 0;

    time_t now = time(NULL);
    if (next_kbd_event == 0) {
        next_kbd_event = (now + 5);
    } else if (kbd_data_index >= strlen(data_sample)) {
        // no data
    } else if (now >= next_kbd_event) {
        m.IN = INT_KEYBOARD;
        m.IO = data_sample[ kbd_data_index ];
        next_kbd_event = (now + 3); // dans 3 secondes
        kbd_data_index = (kbd_data_index + 1);
    }
    return m;
}


/**********************************************************
** Simulation de la CPU (mode utilisateur)
***********************************************************/

PSW simulate_cpu(PSW m) {

    init_cpu();

    /* pas d'interruption */
    m.IN = INT_NONE;

    m = keyboard_event(m);
    if (m.IN) return (m);
    
    /*** lecture et décodage de l'instruction ***/
    WORD value = read_logical_mem(m.PC, &m);
    if (m.IN) return (m);
    
    m.RI = decode_instruction(value);
    
    /*** exécution de l'instruction ***/
    switch (m.RI.op) {
    case INST_SET:
        m = cpu_SET(m);
        break;
    case INST_ADD:
        m = cpu_ADD(m);
        break;
    case INST_SUB:
        m = cpu_SUB(m);
        break;
    case INST_NOP:
        m = cpu_NOP(m);
        break;
    case INST_LOAD:
        m = cpu_LOAD(m);
        break;
    case INST_STORE:
        m = cpu_STORE(m);
        break;
    case INST_JUMP:
        m = cpu_JUMP(m);
        break;
    case INST_IFEQ:
        m = cpu_IFEQ(m);
        break;
    case INST_IFGT:
        m = cpu_IFGT(m);
        break;
    case INST_IFLT:
        m = cpu_IFLT(m);
        break;
    case INST_SYSC:
        m = cpu_SYSC(m);
        break;
    default:
        /*** interruption instruction inconnue ***/
        m.IN = INT_INST;
        return (m);
    }
    
    /*** arrêt si l'instruction a provoqué une interruption ***/
    if (m.IN) return m;

    /*** interruption après chaque instruction ***/
    m.IN = INT_TRACE;
    return m;
}


/**********************************************************
** afficher les registres de la CPU
***********************************************************/

void dump_cpu(PSW m) {
    printf("PC = %3d | ", m.PC);
    printf("AC = %4d | IN = ", m.AC);
    switch (m.IN) {
        case INT_NONE: printf("NONE");break;
        case INT_SEGV: printf("SEGV");break;
        case INT_INST: printf("INST");break;
        case INT_TRACE: printf("TRACE");break;
        case INT_SYSC: printf("SYSC");break;
        case INT_KEYBOARD: printf("KEYB");break;
        default: printf("???");
    }
    printf(" | RI = ");
    switch (m.RI.op) {
        case INST_ADD: printf("ADD %d", m.RI.arg); break;
        case INST_IFEQ: printf("IFEQ %d", m.RI.arg); break;
        case INST_IFGT: printf("IFGT %d", m.RI.arg); break;
        case INST_IFLT: printf("IFLT %d", m.RI.arg); break;
        case INST_JUMP: printf("JUMP %d", m.RI.arg); break;
        case INST_LOAD: printf("LOAD %d", m.RI.arg); break;
        case INST_NOP: printf("NOP"); break;
        case INST_SET: printf("SET %d", m.RI.arg); break;
        case INST_STORE: printf("STORE %d", m.RI.arg); break;
        case INST_SUB: printf("SUB %d", m.RI.arg); break;
        case INST_SYSC: printf("SYSC %d", m.RI.arg); break;
        default: printf("???? %d", m.RI.arg); break;
    }
    printf("\n\n");
}
