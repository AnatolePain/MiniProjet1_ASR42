#include "helpers.h"
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include "types.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

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
            printf("ust(%d)=%3d  ", j, tab[ ( (sizeof(int)*nb_colonnes*i) + ARRAY_SHIFT ) + j ]);            
        }
        printf("\n");
    }
    //printf("\n");
}

void get_semop_values(int* tab,struct semop_values* semop_values, int num_spec){
    int j , i;
    int nb_spec = tab[0];
    int nb_categorie = tab[1];

    semop_values->nsops = 0;
    for(j = 0 ; j < nb_categorie ; j++){
        if (tab[ ( (sizeof(int)*nb_categorie*num_spec) + ARRAY_SHIFT ) + j ]){ //enlève les opérations à zéro ustensiles
            semop_values->sops[semop_values->nsops].sem_num = j;
            semop_values->sops[semop_values->nsops].sem_op = -1*tab[ ( (sizeof(int)*nb_categorie*num_spec) + ARRAY_SHIFT ) + j ];
            semop_values->nsops++;
        }
    }

}

void rendre_ustensiles_values(struct semop_values* semop_values){
    int i;
    for(i = 0 ; i < semop_values->nsops ; i++){
        semop_values->sops[i].sem_op = -1*semop_values->sops[i].sem_op;
    }
}

void afficher_sem(int semid, int nb_categorie){
    int i, val;
	printf("Sem : ");
	for(i = 0 ; i < nb_categorie ; i++){
        val = semctl(semid,i,GETVAL);
        printf("%d, ", val);
	}
	printf("\n");

}