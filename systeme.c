
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
    ASLEEP =  2,            /* thread endormi           */
    GETCHAR = 3,            /* thread en attente de caractère */
} STATE;                    /* État d'un thread         */

typedef struct {
    PSW    cpu;             /* mot d'état du processeur */
    STATE  state;           /* état du thread           */
} PCB;                      /* Un Process Control Block */

PCB thread[MAX_THREADS];    /* table des threads        */

int current_thread = -1;    /* nu du thread courant     */
PSW idle;   
time_t wake_up_times[MAX_THREADS];     
char tampon = '\0';
           


/**********************************************************
** Ajouter une entrée dans le tableau des threads.
**
** 1) trouver une case libre dans le tableau (EMPTY).
** 2) préparer cette case avec le PSW et l'état READY.
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

void new_thread(PSW cpu) {
    int i = 0;
    while(thread[i].state != EMPTY){
        i++;
    }
    thread[i].cpu = cpu;
    thread[i].state = READY;
    printf("New thread created : %d\n", i);
}


/**********************************************************
** Tuer un thread
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

void kill_thread(int p) {
    thread[p].state = EMPTY;
    int remaingin_thread = 0;
    for(int i = 0; i < MAX_THREADS; i++){
        if(thread[i].state == READY){
            remaingin_thread++;
        }
    }
    if(remaingin_thread == 0){
        printf("No more thread to execute, exiting...\n");
        exit(EXIT_SUCCESS);
    }

    
}


/**********************************************************
** Réveiller les threads endormis.
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

void wakeup(void) {
    time_t current_time;
    time(&current_time);
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread[i].state == ASLEEP && difftime(current_time, wake_up_times[i]) >= 0) {
            thread[i].state = READY;
        }
    }
}

/**********************************************************
** Ordonnancer l'exécution des threads.
**
** SUIVEZ L'ORDRE DES QUESTIONS DU SUJET DE TP.
***********************************************************/

PSW scheduler(PSW cpu) {
    assert(&cpu!=NULL);
    PCB current_pcb = { .state=READY, .cpu = cpu};
    thread[current_thread]=current_pcb;
    do {
        current_thread = (current_thread + 1) % MAX_THREADS;
    } while (thread[current_thread].state != READY);
    printf("thread %d \t ",current_thread);

    // Si aucun processus n'est prêt, renvoyer le thread idle
    if (thread[current_thread].state != READY) {
        return idle;
    }

    return thread[current_thread].cpu;
}



/**********************************************************
** Démarrage du système (création d'un code)
***********************************************************/

PSW system_init(void) {
    printf("Booting\n");
    
    /*** création d'un code P1 ***/

    // le code (PC) démarre en 20 et le reste est à zéro
    PSW cpu = { .PC = 20 };
    idle.PC = 120;
    thread[0].state = READY;
    thread[1].state = READY;
    assemble(cpu.PC, "prog1.asm");
    assemble_string(idle.PC, "loop: jump loop");
    
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
    SYSC_NEW_THREAD = 300,   // créer un nouveau thread
    SYSC_SLEEP      = 400,   // dormir (en millisecondes)
    SYSC_GETCHAR    = 500,   // lire un caractère au clavier
};


PSW sysc_exit(PSW cpu) {
    kill_thread(current_thread);
    printf("Thread stopped :  %d\n", current_thread);
    exit(EXIT_SUCCESS);
}


PSW sysc_puti(PSW cpu) {
    printf("Stocked INT in AC : %d\n", cpu.AC);
    return cpu;
}


PSW sysc_putc(PSW cpu) {
    printf("Fonction %s à terminer.\n", __FUNCTION__);
    exit(EXIT_FAILURE);
}


PSW sysc_new_thread(PSW cpu) {
    PSW child = cpu;
    child.AC = 0;
    new_thread(child);
    for(size_t i = 0; i < MAX_THREADS; i++ ){
        if(cpu.PC == thread[i].cpu.PC && cpu.AC == thread[i].cpu.AC && cpu.IN == thread[i].cpu.IN && cpu.RI.arg == thread[i].cpu.RI.arg && cpu.RI.op == thread[i].cpu.RI.op && cpu.IO == thread[i].cpu.IO){
            thread[i].cpu.AC=1;
            break;
        }
    }
    return cpu;
}


PSW sysc_sleep(PSW cpu) {
    int sleep_time = cpu.AC;
    thread[current_thread].state = ASLEEP;
    time(&wake_up_times[current_thread]);
    wake_up_times[current_thread] += sleep_time;
    return scheduler(cpu);
}



PSW sysc_getchar(PSW cpu) {
    if (tampon == '\0') {
        thread[current_thread].state = GETCHAR;
        return scheduler(cpu);
    } else {
        cpu.AC = tampon;
        tampon = '\0';
        return cpu;
    }
}


void keyboard_event(void) {
    int char_thread = -1;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread[i].state == GETCHAR) {
            char_thread = i;
            break;
        }
    }

    if (char_thread != -1) {
        thread[char_thread].cpu.AC = 'A';  
        thread[char_thread].state = READY;
    } else {
        tampon = 'A';  
    }
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
        case SYSC_NEW_THREAD:
            cpu = sysc_new_thread(cpu);
        
        case SYSC_SLEEP:
            cpu = sysc_sleep(cpu);
            break;

        case SYSC_GETCHAR:
            cpu = sysc_getchar(cpu);
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
            scheduler(cpu);
        case INT_SYSC:
            cpu = system_call(cpu);
            break;
        case INT_KEYBOARD:
            printf("Interruption clavier\n");
            break;
        default:
            break;
    }
    return cpu;
}
