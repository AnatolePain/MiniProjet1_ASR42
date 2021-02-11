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

#define NB_CLIENT 5
#define NB_ARG_FIXE 5

//git submodule update --remote --merge
//handler de fonction 
//bon objet ipc

int file_mess_clients_serveurs; //faire un singleton pour que ce soit plus propre ? 
int file_mess_serveurs_cuisiniers;
//int* nb_ustensiles;

void sig_handler(int signo){
	msgctl(file_mess_serveurs_cuisiniers,IPC_RMID,NULL);
	msgctl(file_mess_clients_serveurs,IPC_RMID,NULL);
	//free(nb_ustensiles);
	printf("fermeture du fast-food\n");
	exit(0);
	assert(0);			
}

int main(int argc,char * argv[])
{
	int i, j,
		nb_serveurs, nb_cuisiniers, nb_term, nb_spec,
		shmid;
	char buf[256];
	char buf2[256];
	key_t cle; /* cle de la file     */
	int* nb_ustensiles;
	int** specialite;

	carte* stock_fastfood;

	int nb_categorie = argc - NB_ARG_FIXE;

	srand(getpid());
	printf("argc = %d\n",argc );
	printf("nb_categorie = %d\n",nb_categorie );
	/*
	int shmid,
	    i,
	    *count,
	    semid;
	
	char buf[256];
	key_t k = ftok("/tmp",1);
	assert(k!=-1);

	shmid=shmget(k, sizeof(int), IPC_CREAT|0666);
	assert(shmid >= 0);

	count = (int*)shmat(shmid,NULL,0);
	assert(count != (void*)-1);

	semid = semget(k,2,0666 | IPC_CREAT | IPC_EXCL);
	assert(sid < 0);

	struct sembuf p = {0,-1, 0};
	struct sembuf v = {0, 1, 0};

	*count = 0;
	*/

	if(argc <= NB_ARG_FIXE ){
		fprintf(stderr, "USAGE: %s nb_serveurs nb_cuisiniers nb_term nb_spec nb_1 nb_2 ... nb_k\n", argv[0]);
		exit(0);
	}


	nb_ustensiles = (int*)malloc(nb_categorie*sizeof(int));

	nb_serveurs = (int)strtol(argv[1],NULL,10);
	nb_cuisiniers = (int)strtol(argv[2],NULL,10);
	nb_term = (int)strtol(argv[3],NULL,10);
	nb_spec = (int)strtol(argv[4],NULL,10);

	for(int i = 0 ; i < nb_categorie; i++){
		nb_ustensiles[i] = (int)strtol(argv[i+NB_ARG_FIXE],NULL,10);
	}

	for(int i = 0 ; i < nb_categorie ; i++){
		printf("nb_ustensiles[%d] = %d\n",i,nb_ustensiles[i]);
	}






	//nb_clients = NB_CLIENT;

	assert(set_signal_handler(SIGINT,sig_handler)==0);

	//setvbuf(stdin, NULL, _IONBF, 0);
	//setvbuf(stdout, NULL, _IONBF, 0);


	//====== CREATION DES FILES  DE MESSAGE CLIENT <--> SERVEUR ========

	/* cacul de la cle de la file    */
	cle = ftok("client_serveur_key",1);
	assert(cle != -1);

	/* Creation file de message :    */
	file_mess_clients_serveurs = msgget(cle,0666 | IPC_CREAT);	
	assert(file_mess_clients_serveurs != -1);

	//==================================================================

	//====== CREATION DES FILES  DE MESSAGE SERVEUR <--> CUISINER ========

	/* cacul de la cle de la file    */
	cle = ftok("serveur_cuisinier_key",1);
	assert(cle != -1);

	/* Creation file de message :    */
	file_mess_serveurs_cuisiniers = msgget(cle,0666 | IPC_CREAT);	
	assert(file_mess_serveurs_cuisiniers != -1);

	//===================================================================

	//========== Initialisation du segment partagé =========

	specialite = (int**)malloc(nb_spec*sizeof(int*));
	for(i = 0 ; i < nb_spec ; i++){
		specialite[i] = (int*)malloc(nb_categorie*sizeof(int*)  );
	}

	for(i = 0 ; i < nb_spec ; i++){
		for(j = 0 ; j < nb_categorie ; j++){
			specialite[i][j] = rand()%(nb_ustensiles[j]+1);
		}
	}

	for(i = 0 ; i < nb_spec ; i++){
		for(j = 0 ; j < nb_categorie ; j++){
			printf("%d, ",specialite[i][j]);
		}
		printf("\n");
	}

	cle = ftok("share_memory",1);
	assert(cle!=-1);

	shmid=shmget(cle, sizeof(carte), IPC_CREAT|0666);
	assert(shmid >= 0);

	stock_fastfood = (carte*)shmat(shmid,NULL,0);
	assert(stock_fastfood != (void*)-1);

	stock_fastfood->nb_specialite = nb_spec;
	stock_fastfood->nb_categorie = nb_categorie;
	stock_fastfood->specialite = specialite;
	stock_fastfood->nb_ustensiles = nb_ustensiles;

	// free(specialite);
	// free(nb_ustensiles);

	printf("=========================\n\n");

	for(i = 0 ; i < stock_fastfood->nb_specialite ; i++){
		for(j = 0 ; j < stock_fastfood->nb_categorie ; j++){
			printf("%d, ",stock_fastfood->specialite[i][j]);
		}
		printf("\n");
	}

	//semid = semget(,2,0666 | IPC_CREAT | IPC_EXCL);
	//assert(sid < 0);


	//CLIENT
	printf("== Création des client == \n");
	for (i=1;i<=nb_serveurs;i++){
		pid_t p = fork();
		assert( p != -1);

		if (p==0) {
			snprintf(buf,sizeof(buf),"%d",i);
			snprintf(buf2,sizeof(buf2),"%d",nb_serveurs);
			execl("./client","./client",buf,buf2,NULL);
			assert(0);
		}
	}
	sleep(2);
	printf("== Création client términé == \n");

	//CUISINIERS
	sleep(3);
	printf("== Création des cuisinier == \n");
	for (i=1;i<=nb_cuisiniers;i++){
		pid_t p = fork();
		assert( p != -1);

		if (p==0) {
			snprintf(buf,sizeof(buf),"%d",i);

			execl("./cuisinier","./cuisinier",buf,NULL);
			assert(0);
		}
	}
	sleep(2);
	couleur(REINIT);
	printf("== Création des cuisinier términé == \n");

	//SERVEURS
	sleep(3);
	printf("== Création des serveurs ==\n");
	for (i=1;i<=NB_CLIENT;i++){
		pid_t p = fork();
		assert( p != -1);

		if (p==0) {
			snprintf(buf,sizeof(buf),"%d",i);
			execl("./serveur","./serveur",buf,NULL);
			assert(0);
		}
	}
	sleep(2);
	couleur(REINIT);
	printf("== Création des serveurs términé == \n");


	for (i=1;i<=(NB_CLIENT+nb_cuisiniers+nb_serveurs);i++) wait(NULL);


	printf("j\'ai fini d'attendre tout mes fils...\n");
	couleur(REINIT);

	//assert(shmctl(shmid,IPC_RMID,0) >=0);

	free(nb_ustensiles);

}


	

