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

#define NB_CLIENT 2
#define NB_ARG_FIXE 5

int file_mess_clients_serveurs; //faire un singleton pour que ce soit plus propre ? 
int file_mess_serveurs_cuisiniers;
int shmid, semid;
//int* nb_ustensiles;

// int getfile_memessage(){
// 	static int file_mess = -1;
// 	if(file_mess == -1){
// 		file_mess = msgget(cle,0666 | IPC_CREAT);	
// 		assert(file_mess != -1);
// 	}else{
// 		return file_mess;
// 	}
// }

void sig_handler(int signo){
	msgctl(file_mess_serveurs_cuisiniers,IPC_RMID,NULL);
	msgctl(file_mess_clients_serveurs,IPC_RMID,NULL);
	assert(shmctl(shmid,IPC_RMID,0) >=0);
	assert(semctl(semid,0,IPC_RMID) >=0);
	//free(nb_ustensiles);
	printf("fermeture du fast-food\n");
	exit(0);
	assert(0);			
}

int main(int argc,char * argv[])
{
	int i, j, err,
		nb_serveurs, nb_cuisiniers, nb_term, nb_spec;
	char buf[256], buf2[256], buf3[256], buf4[256];
	key_t cle; /* cle de la file     */
	ushort* nb_ustensiles;
	int** specialite;

	ushort semvals[25];//test (à enlever)

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

	nb_ustensiles = (ushort*)malloc(nb_categorie*sizeof(ushort));
	for(int i = 0 ; i < nb_categorie; i++){
		nb_ustensiles[i] = (ushort)strtol(argv[i+NB_ARG_FIXE],NULL,10);
	}

	assert(set_signal_handler(SIGINT,sig_handler)==0);

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

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

	int rand1, rand2;

	for(i = 0 ; i < nb_spec ; i++){
        for(j = 0 ; j < nb_categorie ; j++){
			rand1 = rand()%(nb_ustensiles[j]+1);
			//rand2 = (int) (rand1/2);
            carte[ ( (sizeof(int)*nb_categorie*i) + ARRAY_SHIFT ) + j]  = rand1 ;            
        }
    }

	printf("Carte : \n");
	afficher_carte(carte);
	printf("\n");



	/* ===== Sémaphore ===== */

	cle = ftok("sem",1);
	assert(cle!=-1);

	semid = semget(cle,nb_categorie,0666 | IPC_CREAT);
	assert(semid!=1);

	//remplir
	arg.array = nb_ustensiles;
	semctl(semid, 0, SETALL, arg);

	//afficher(à enlever)
	afficher_sem(semid, nb_categorie);


	/* ========= Création des processus ========== */

	//CUISINIERS
	sleep(1);
	printf("\n== Création des cuisinier == \n");
	for (i=0;i<nb_cuisiniers;i++){
		pid_t p = fork();
		assert( p != -1);

		if (p==0) {
			snprintf(buf,sizeof(buf),"%d",i);
			snprintf(buf2,sizeof(buf2),"%d",nb_spec);
			snprintf(buf3,sizeof(buf3),"%d",nb_categorie);
			snprintf(buf4,sizeof(buf4),"%d",nb_serveurs);
			execl("./cuisinier","./cuisinier",buf,buf2,buf3,buf4,NULL);
			assert(0);
		}
	}
	sleep(1);
	couleur(REINIT);

	//SERVEURS
	sleep(1);
	printf("== Création des serveurs ==\n");
	for (i=0;i<nb_serveurs;i++){
		pid_t p = fork();
		assert( p != -1);

		if (p==0) {
			snprintf(buf,sizeof(buf),"%d",i);
			snprintf(buf2,sizeof(buf2),"%d",nb_serveurs);
			execl("./serveur","./serveur",buf,buf2,NULL);
			assert(0);
		}
	}
	sleep(1);
	couleur(REINIT);

	//CLIENT
	while(1){
		couleur(REINIT);
		printf("== SALVE CLIENT== \n");
		for (i=0;i<NB_CLIENT;i++){
			sleep(1);
			pid_t p = fork();
			assert( p != -1);

			if (p==0) {
				snprintf(buf,sizeof(buf),"%d",i);
				snprintf(buf2,sizeof(buf2),"%d",nb_serveurs);
				snprintf(buf3,sizeof(buf3),"%d",nb_spec);
				execl("./client","./client",buf,buf2,buf3,NULL);
				assert(0);
			}
		}
		sleep(3);
	}

	// free(nb_ustensiles);

	for (i=1;i<=(NB_CLIENT+nb_cuisiniers+nb_serveurs);i++) wait(NULL);
	
	printf("j\'ai fini d'attendre tout mes fils...\n");
	couleur(REINIT);
}


	

