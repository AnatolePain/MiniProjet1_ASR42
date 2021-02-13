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

//#include <bits/msq.h>

#include "types.h"

int main(int argc, char const *argv[])
{
	int type = (int)strtol(argv[1],NULL,10);
	int nb_serveurs = (int)strtol(argv[2],NULL,10); 
	int nb_spec = (int)strtol(argv[3],NULL,10); 
	int pid = getpid();
	key_t cle; /* cle de la file     */
	int file_mess;		/* ID de la file    */
	requete_t requete;
	int nb_message_file;

	couleur(ROUGE);
	fprintf(stdout, "Je suis le client numéro %d \n", pid );
	couleur(REINIT);

	srand(pid);

	/* creation de la requete :          */
	requete.type = (rand()%nb_serveurs);  //choisie aléatiorement un des serveurs
	requete.expediteur = pid;
	requete.num_specialite = rand()%nb_spec;

	cle = ftok("client_serveur_key",1);
	assert(cle != -1);

	/* Recuperation file de message :    */
	file_mess = msgget(cle,0);	
	assert(file_mess != -1);

	struct msqid_ds buf;
	msgctl(file_mess,IPC_STAT,&buf);
	nb_message_file = (int)(buf.msg_qnum);

	/* envoi de la requete :             */
	couleur(ROUGE);
	fprintf(stdout, " Le client %d à choisie la file %d, il attends un serveur \n", pid, (int)requete.type );
	couleur(REINIT);
	if( msgsnd(file_mess,&requete,sizeof(requete)-sizeof(requete.type),0) == -1 ){
		fprintf(stderr, "erreur: %d\n", errno);
	}
	
	couleur(ROUGE);
	fprintf(stdout, " Le client %d vient d'envoyer la spécialité %d à la file de type %d \n", pid, requete.num_specialite, (int)requete.type );
	couleur(REINIT);

	return EXIT_SUCCESS;
}