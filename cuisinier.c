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
	int pid = getpid();

	int i, j, shmid;
	couleur(JAUNE);
	fprintf(stdout,"Je suis le cuisinier numéro %d | pid : %d \n", type, pid  );
	couleur(REINIT);
	carte* stock_fastfood;

	key_t cle; /* cle de la file     */
	int file_mess;		/* ID de la file    */
	requete_t requete;

	cle = ftok("serveur_cuisinier_key",1);
	assert(cle != -1);

	/* Recuperation file de message :    */
	file_mess = msgget(cle,0);	
	assert(file_mess != -1);

	assert(set_signal_handler(SIGINT,sig_handler)==0);

	cle = ftok("share_memory",1);
	assert(cle!=-1);

	shmid=shmget(cle, sizeof(carte), 0);
	assert(shmid >= 0);

	stock_fastfood = (carte*)shmat(shmid,NULL,0);
	assert(stock_fastfood != (void*)-1);

	couleur(JAUNE);
	fprintf(stdout,"TEST SHARE MEMORY BEGIN...\n");
	for(i = 0 ; i < stock_fastfood->nb_specialite ; i++){
		for(j = 0 ; j < stock_fastfood->nb_categorie ; j++){
			printf("%d, ",stock_fastfood->specialite[i][j]);
		}
		printf("\n");
	}
	fprintf(stdout,"TEST SHARE MEMORY END...\n");
	couleur(REINIT);

	/* attente de la requete fait par le client :           */

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