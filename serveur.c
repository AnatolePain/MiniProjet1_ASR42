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
	couleur(CYAN);
	fprintf(stdout,"fermeture des serveurs\n");
	couleur(REINIT);
	exit(0);
	assert(0);			
}

int main(int argc, char const *argv[])
{

	int type = (int)strtol(argv[1],NULL,10);
	int pid = getpid();
	key_t cle_client_serveur; /* cle de la file     */
	key_t cle_serveur_cuisinier; /* cle de la file     */
	int file_mess_client_serveur;		/* ID de la file    */
	int file_mess_serveur_cuisinier;		/* ID de la file    */
	requete_t requete;

	couleur(CYAN);
	fprintf(stdout,"Je suis le serveurs numéro %d | je m'occupe de la file %d \n", pid, type );
	couleur(REINIT);

	cle_client_serveur = ftok("client_serveur_key",1);
	assert(cle_client_serveur != -1);
	cle_serveur_cuisinier = ftok("serveur_cuisinier_key",1);
	assert(cle_serveur_cuisinier != -1);

	/* Recuperation file de message :    */
	file_mess_client_serveur = msgget(cle_client_serveur,0);	
	assert(file_mess_client_serveur != -1);
	file_mess_serveur_cuisinier = msgget(cle_serveur_cuisinier,0);	
	assert(file_mess_serveur_cuisinier != -1);

	assert(set_signal_handler(SIGINT,sig_handler)==0);

	/* attente de la requete fait par le client :           */

	while(1){
		/*récéption message */
		couleur(CYAN);
		fprintf(stdout,"Le serveurs numéro %d attend des clients dans la file de type %d \n\n", pid, type );
		couleur(REINIT);
		if ( msgrcv(file_mess_client_serveur,&requete,sizeof(requete)-sizeof(requete.type),type,0) == -1 ){
			fprintf(stderr, "erreur: %d\n", errno);
		}
		couleur(CYAN);
		fprintf(stdout, " Le serveur %d reçoit le numéro de spécialité %d provonant du client %d\n",pid,requete.num_specialite, requete.expediteur);
		couleur(REINIT);

		/*envoir dans la file des cuisiniers*/
		if( msgsnd(file_mess_serveur_cuisinier,&requete,sizeof(requete)-sizeof(requete.type),0) == -1 ){
			fprintf(stderr, "erreur: %d\n", errno);
		}
		couleur(CYAN);
		fprintf(stdout, " Le serveur %d vient d'envoyer la spécialité %d du client %d à la file d'attente des cuisiners\n",pid, requete.num_specialite, requete.expediteur );
		couleur(REINIT);

		sleep(2);
	}

	return EXIT_SUCCESS;
}