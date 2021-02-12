#include "helpers.h"
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include "types.h"

int set_signal_handler(int signo, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;  
    sigemptyset(&sa.sa_mask);  
    sa.sa_flags = SA_RESTART;       
    return sigaction(signo, &sa, NULL);
}

void afficher_carte(int* tab){

    int j , i;
    int nb_lignes = tab[0];
    int nb_colonnes = tab[1];

    for(i = 0 ; i < nb_lignes ; i++){
        printf("Specialite(%d) : ",i);
        for(j = 0 ; j < nb_colonnes ; j++){
            printf("ust(%d)=%3d  ", j+ARRAY_SHIFT, tab[ ( (sizeof(int)*nb_colonnes*i) + ARRAY_SHIFT ) + j ],i);            
        }
        printf("\n");
    }
    printf("\n");
}