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
	int i, j, err,
		nb_serveurs, nb_cuisiniers, nb_term, nb_spec,
		shmid, semid;
	char buf[256], buf2[256], buf3[256];
	key_t cle; /* cle de la file     */
	int* nb_ustensiles;
	int** specialite;

	ushort semvals[25];//test

	union semun  {
		int val;
		struct semid_ds *buf;
		ushort *array;
	} arg;

	int* carte;
	int nb_categorie = argc - NB_ARG_FIXE;

	if(argc <= NB_ARG_FIXE ){
		fprintf(stderr, "USAGE: %s nb_serveurs nb_cuisiniers nb_term nb_spec nb_1 nb_2 ... nb_k\n", argv[0]);
		exit(0);
	}

	nb_serveurs = (int)strtol(argv[1],NULL,10);
	nb_cuisiniers = (int)strtol(argv[2],NULL,10);
	nb_term = (int)strtol(argv[3],NULL,10);
	nb_spec = (int)strtol(argv[4],NULL,10);

	nb_ustensiles = (int*)malloc(nb_categorie*sizeof(int));
	for(int i = 0 ; i < nb_categorie; i++){
		nb_ustensiles[i] = (int)strtol(argv[i+NB_ARG_FIXE],NULL,10);
	}

	assert(set_signal_handler(SIGINT,sig_handler)==0);

	//setvbuf(stdin, NULL, _IONBF, 0);
	//setvbuf(stdout, NULL, _IONBF, 0);

	/* ===== CREATION DES FILES  DE MESSAGE CLIENT <--> SERVEUR ======= */
	cle = ftok("client_serveur_key",1);
	assert(cle != -1);

	file_mess_clients_serveurs = msgget(cle,0666 | IPC_CREAT);	
	assert(file_mess_clients_serveurs != -1);


	/* ===== CREATION DES FILES  DE MESSAGE SERVEUR <--> CUISINER ===== */
	cle = ftok("serveur_cuisinier_key",1);
	assert(cle != -1);

	file_mess_serveurs_cuisiniers = msgget(cle,0666 | IPC_CREAT);	
	assert(file_mess_serveurs_cuisiniers != -1);


	/* ===== Initialisation du segment partagé ======================== */
	cle = ftok("share_memory",1);
	assert(cle!=-1);

	shmid=shmget(cle, nb_categorie*nb_spec*sizeof(int) + ARRAY_SHIFT*sizeof(int), IPC_CREAT|0666);
	if(shmid == -1){
		fprintf(stderr, "erreur: %d\n", errno);
		exit(0);
	}
	//assert(shmid >= 0);

	carte = (int*)shmat(shmid,NULL,0);
	assert(carte != (void*)-1);

	carte[0] = nb_spec;
	carte[1] = nb_categorie;

	for(i = 0 ; i < nb_spec ; i++){
        for(j = 0 ; j < nb_categorie ; j++){
            carte[ ( (sizeof(int)*nb_categorie*i) + ARRAY_SHIFT ) + j]  = rand()%(nb_ustensiles[j]+1);            
        }
    }

	printf("\ncarte : \n");
	afficher_carte(carte);


	/* ===== Sémaphore ===== */

	cle = ftok("sem",1);
	assert(cle!=-1);

	semid = semget(cle,nb_categorie,0666 | IPC_CREAT);
	assert(semid!=1);

	for(i = 0 ; i < nb_categorie ; i++){
		arg.val = nb_ustensiles[i];
		err = semctl(semid,i,SETVAL,arg);
		if(err == -1){
			fprintf(stderr, "erreur semctl (SETVAL): %d\n", errno);
			//exit(0);
		}
	}

	//assert(semctl(semid,0,GETALL,arg)!=-1);
	arg.array = semvals;
	semctl(semid,0,GETALL,arg);
	if(err == -1){
		fprintf(stderr, "erreur semctl (GETALL): %d\n", errno);
		//exit(0);
	}

	printf("sem : ");
	for(i = 0 ; i < nb_categorie ; i++){
		printf("%d, ",semvals[i]);
	}
	printf("\n");







	/* ========= Création des processus ========== */
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
			snprintf(buf2,sizeof(buf2),"%d",nb_spec);
			snprintf(buf3,sizeof(buf3),"%d",nb_categorie);
			execl("./cuisinier","./cuisinier",buf,buf2,buf3,NULL);
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

	err = semctl(semid,0,IPC_RMID);
	if( err == -1){
		fprintf(stderr, "erreur: %d\n", errno);
	}
	//assert(semctl(semid,0,IPC_RMID) >=0);

	free(nb_ustensiles);

	for (i=1;i<=(NB_CLIENT+nb_cuisiniers+nb_serveurs);i++) wait(NULL);
	
	printf("j\'ai fini d'attendre tout mes fils...\n");
	couleur(REINIT);
}


	

