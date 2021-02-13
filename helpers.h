#ifndef _HELPERS_H
#define _HELPERS_H

#define NB_CATEGORIE_MAX 100

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


struct semop_values {
    struct sembuf sops[NB_CATEGORIE_MAX];
    int nsops;
};

int set_signal_handler(int signo, void (*handler)(int));
void afficher_carte(int* tab);
void get_semop_values(int* tab,struct semop_values* semop_values, int num_spec);
void afficher_sem(int semid, int nb_categorie);
void rendre_ustensiles_values(struct semop_values* semop_values);

#endif