
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "cpu.h"
#include "asm.h"
#include "systeme.h"


/**********************************************************
** Structures de données de l'ordonnanceur (représentation
** d'un ensemble de threads).
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

#define MAX_THREADS  (20)   /* nb maximum de threads    */

typedef enum {
    EMPTY =   0,            /* thread non-prêt          */
    READY =   1,            /* thread prêt              */
} STATE;                    /* État d'un thread         */

typedef struct {
    PSW    cpu;             /* mot d'état du processeur */
    STATE  state;           /* état du thread           */
} PCB;                      /* Un Process Control Block */

PCB thread[MAX_THREADS];    /* table des threads        */

int current_thread = -1;    /* nu du thread courant     */


/**********************************************************
** Ajouter une entrée dans le tableau des threads.
**
** 1) trouver une case libre dans le tableau (EMPTY).
** 2) préparer cette case avec le PSW et l'état READY.
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

void new_thread(PSW cpu) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}


/**********************************************************
** Tuer un thread
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

void kill_thread(int p) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}


/**********************************************************
** Réveiller les threads endormis.
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

void wakeup(void) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}


/**********************************************************
** Ordonnancer l'exécution des threads.
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

PSW scheduler(PSW cpu) {
    printf("Fonction scheduler() à terminer.\n");
    return cpu;
}


/**********************************************************
** Démarrage du système (création d'un code)
***********************************************************/

PSW system_init(void) {
    printf("Booting\n");
    
    /*** création d'un code P1 ***/
    
    // le code (PC) démarre en 20 et le reste est à zéro
    PSW cpu = { .PC = 20 };
    
    assemble(cpu.PC, "prog1.asm");
    
    return cpu;
}


/**********************************************************
** Traitement des appels au système
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

enum {
    SYSC_EXIT       = 100,   // fin du thread courant
    SYSC_PUTI       = 200,   // afficher le contenu de AC
};


PSW sysc_exit(PSW cpu) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}


PSW sysc_puti(PSW cpu) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}


PSW sysc_putc(PSW cpu) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}


PSW sysc_new_thread(PSW cpu) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}


PSW sysc_sleep(PSW cpu) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}



PSW sysc_getchar(PSW cpu) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}




static PSW system_call(PSW cpu) {
    // suivant l'argument de sysc ARG
    switch (cpu.RI.arg) {
        case SYSC_EXIT:
            cpu = sysc_exit(cpu);
            break;
        case SYSC_PUTI:
            cpu = sysc_puti(cpu);
            break;
        default:
            printf("Appel système inconnu %d\n", cpu.RI.arg);
            break;
    }
    return cpu;
}


/**********************************************************
** Traitement des interruptions par le système (mode système)
***********************************************************/
PSW process_interrupt(PSW cpu) {
    switch (cpu.IN) {
        case INT_SEGV:
            printf("Erreur: Segmentation Violation\n");
            exit(EXIT_FAILURE);
            break;
        case INT_INST:
            printf("Erreur: Instruction Invalide\n");
            exit(EXIT_FAILURE);
            break;
        case INT_TRACE: 
            dump_cpu(cpu); sleep(1);
            break;
        case INT_SYSC:
            printf("Appel système\n");
            cpu = system_call(cpu);
            break;
        case INT_KEYBOARD:
            printf("Interruption clavier\n");
            exit(EXIT_FAILURE); 
            break;
        default:
            break;
    }
    return cpu;
}
