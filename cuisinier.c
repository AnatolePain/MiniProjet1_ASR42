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
	int pid = getpid();
	int cle2;
	int i, j, shmid;
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

	while(1){
		//récéption message 
		if ( msgrcv(file_mess,&requete,sizeof(requete)-sizeof(requete.type),0,0) == -1 ){
			fprintf(stderr, "erreur: %d\n", errno);
		}

		couleur(JAUNE);
		fprintf(stdout, " Le cuisiniers %d reçoit l'ordre de faire la spécialité %d\n", pid, requete.num_specialite );
		couleur(REINIT);

	}


	return EXIT_SUCCESS;
}