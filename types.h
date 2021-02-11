#ifndef _TYPES_H
#define _TYPES_H

#define couleur(param) fprintf(stdout,"\033[%sm",param)

#define NOIR  "30"
#define ROUGE "31"
#define VERT  "32"
#define JAUNE "33"
#define BLEU  "34"
#define CYAN  "36"
#define BLANC "37"
#define REINIT "0"

typedef struct 
{
	long type;
	int num_specialite;
	pid_t expediteur;
} 
requete_t;

typedef struct 
{
	int nb_specialite;
	int nb_categorie;
	int** specialite;
	int* nb_ustensiles;
}
carte;


#endif