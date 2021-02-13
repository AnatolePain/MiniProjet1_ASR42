#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/msg.h>

#include "types.h"
#include "helpers.h"

void sig_handler(int signo){
	couleur(JAUNE);
	fprintf(stdout,"fermeture des cuisinier\n");
	couleur(REINIT);
	exit(0);
	assert(0);			
}

int main(int argc, char const *argv[])
{
	int type = (int)strtol(argv[1],NULL,10);
	int nb_spec = (int)strtol(argv[2],NULL,10);
	int nb_categorie = (int)strtol(argv[3],NULL,10);
	int nb_serveurs = (int)strtol(argv[4],NULL,10);
	int pid = getpid();
	int cle2;
	int err;
	int i, j, shmid, semid;
	int* carte;
	key_t cle; /* cle de la file     */
	int file_mess;		/* ID de la file    */
	requete_t requete;

	assert(set_signal_handler(SIGINT,sig_handler)==0);

	couleur(JAUNE);
	fprintf(stdout,"Je suis le cuisinier numéro %d | pid : %d \n", type, pid  );
	couleur(REINIT);

	/* ===== File de messages ===== */
	cle = ftok("serveur_cuisinier_key",1);
	assert(cle != -1);

	file_mess = msgget(cle,0);	
	assert(file_mess != -1);

	


	/* ===== Mémoire partagé ===== */
	cle2 = ftok("share_memory",1);
	assert(cle2!=-1);
	shmid=shmget(cle2, nb_categorie*nb_spec*sizeof(int) + ARRAY_SHIFT*sizeof(int), 0);
	if(shmid == -1){
		fprintf(stderr, "erreur: %d\n", errno);
		exit(0);
	}
	//assert(shmid >= 0);
	
	carte = (int*)shmat(shmid,NULL,0);
	assert(carte != (void*)-1);

	//affichage
	couleur(JAUNE);
	afficher_carte(carte);
	couleur(REINIT);

	/* ===== sémaphore ===== */
	cle = ftok("sem",1);
	assert(cle!=-1);

	semid = semget(cle,0,0);
	assert(semid!=1);

	while(1){
		//récéption message 
		couleur(JAUNE);
		fprintf(stdout,"Le cuisinier %d attends une specialité a faire dans la file des cuisiniers (type entre 0 et %d) \n\n",pid,nb_serveurs-1);
		couleur(REINIT);
		//Chaque serveur à un type dans l'odres 0..1..2.. Il ne lit pas les types au dessus.
		if ( msgrcv(file_mess,&requete,sizeof(requete)-sizeof(requete.type),-nb_serveurs,0) == -1 ){
			fprintf(stderr, "erreur: %d\n", errno);
		}

		struct semop_values semop_values;
		get_semop_values(carte,&semop_values,requete.num_specialite);

		couleur(JAUNE);
		printf("Le cuisinier %d veut prendre les ustensiles ",pid);
		for(i = 0 ; i < semop_values.nsops ; i++){
			printf("%d(%d), ", semop_values.sops[i].sem_op, semop_values.sops[i].sem_num);	
		}
		printf("pour faire la specialité %d\n",requete.num_specialite);
		couleur(REINIT);
		
		err = semop(semid,semop_values.sops,semop_values.nsops);
		if(err == -1){
			fprintf(stderr, "erreur: %d\n", errno);
			//exit(0);
		}

		couleur(JAUNE);
		printf("Le cuisinier %d a pue prendre tout les ustensiles qu'il voulait, donc il reste dans la cuisine les ustensiles suivant:  ",pid);
		afficher_sem(semid,nb_categorie);
		printf("(%d) Il se met au travaille...\n\n",pid);
		couleur(REINIT);

		rendre_ustensiles_values(&semop_values);

		err = semop(semid,semop_values.sops,semop_values.nsops);
		if(err == -1){
			fprintf(stderr, "erreur: %d\n", errno);
			//exit(0);
		}

		couleur(VERT);
		printf("... le cuisinier %d a fini de trvailler, il repose tout ses ustensiles. Il reste dans la cuisine les ustensiles suivant: ",pid);
		afficher_sem(semid,nb_categorie);
		couleur(REINIT);

		requete.type += nb_serveurs;

		couleur(VERT);
		printf("Le cuisinier %d a fini de cuisiner, il notifie le serveur qui gère la file %d (type + nombres de serveurs)\n",pid,(int)requete.type);
		couleur(REINIT);

		if( msgsnd(file_mess,&requete,sizeof(requete)-sizeof(requete.type),0) == -1 ){
			fprintf(stderr, "erreurTEST1: %d\n", errno);
		}


	}


	return EXIT_SUCCESS;
}