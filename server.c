/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include "dico.c"
#include "dico.h"
#include <netdb.h> 		/* pour hostent, servent */


#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

struct Sthread{
    int socket; // le socket géré 
    int * etat; /* Variable état partie 0 -début 1 encours 2 fin*/
    int numJoueur; // pointeur vers num du joueur
    int * joueurs_prets;
}typedef thread;

//PARAMETRES
int nb_joueurs_requis = 2 ;

//VARIABLES GLOBALES
int nbj_rejoins = 0;
int sockets[2];
int socket_descriptor ; 	/* descripteur de socket */
int longueur_buffer; 		/* longueur d'un buffer utilisé */
char motSecret[100] = {0}; // Ce sera le mot à trouver
int *lettreTrouvee = NULL; // Un tableau de booléens. Chaque case correspond à une lettre du mot secret. 0 = lettre non trouvée, 1 = lettre trouvée
int jeuEnCours = 1;
long remaining_life = 10; // Compteur de coups restants (0 = mort)

//DECLARATION DE FONCTION
int gagne(int lettreTrouvee[], long tailleMot);
int rechercheLettre(char lettre, char motSecret[], int lettreTrouvee[]);
void * pendu(void * params);
void msg_all(int socks[], char * message);

//fonction pour msg à tous les joueurs
void msg_all(int socks[5],char * msg){
  for(int i =0;i< nb_joueurs_requis ; i++){
    write(socks[i],msg,strlen(msg));
  }
}

void * pendu(void* params)
{
    thread* param = params;
    char lettre = 0; // Stocke la lettre proposée par l'utilisateur (retour du scanf)
    char pseudo[32];
    int pseudoEnvoye=0;
    char buffer[256];

    //printf("Debug :  Pseudo ok = %d\n",*(*param).pseudoOK);
   
     //début du programme 
    while(pseudoEnvoye == 0){
        read((* param).socket,buffer,sizeof(buffer));
        strcpy(pseudo,buffer);
        printf("%s---Pseudo choisit : %s\n",pseudo,pseudo);
        (*(*param).joueurs_prets) += 1;
        //printf("Pseudo ok dans while %d\n",*(*param).pseudoOK);
        pseudoEnvoye=1;   
    }
    //printf("Debug : Pseudo ok après while = %d\n",*(*param).pseudoOK);
            
    printf("%s---En attente des autres joueurs---\n",pseudo);
    while(*(* param).etat == 0){
    
    }
    printf("%s---Je commence le jeu !---\n",pseudo);
    
    int taille_message = strlen(motSecret)+1+2+1+1+1;
	char message[taille_message];
    // On continue à jouer tant qu'il reste au moins un coup à jouer ou qu'on à pas gagner
    while (jeuEnCours)
    {
            if(read((* param).socket, buffer, 1)<0){
            	remaining_life = -2;
            	break;
            }
            lettre = toupper((buffer[0]));
            printf("%s --- : Lettre recue : %s\n",pseudo,&lettre);
            
            if (!rechercheLettre(lettre, motSecret, lettreTrouvee)){
                remaining_life--;
            }
            
            //Après avoir mis à jour le mot secret, on notifie tous les client du nouvel etat de la partie
            // Structure d'un message : "mot_codé vie etat" où etat : 0:Encours 1:Perdu 2:Gagné
			int char_to_write = 0;

            //On recopie le mot secret dans le message à envoyer
            for (int i = 0 ; i < strlen(motSecret) ; i++){
            	if (lettreTrouvee[i]) {
            		message[i] = motSecret[i];
            	}else{
            		message[i] = '*';
            	}
            	char_to_write++;
            }
			
			//On ajoute au message les points de vie restant
            char life_char[3];
            if(remaining_life<10){
            	life_char[0] = '0';
            	snprintf(&life_char[1],sizeof(&life_char[1]),"%d",(int)remaining_life);
            }else{
            	snprintf(life_char,sizeof(life_char),"%d",(int)remaining_life);
            }
            
            message[char_to_write] = ' ';
            char_to_write++;
            message[char_to_write] = life_char[0];
            char_to_write++;
            message[char_to_write] = life_char[1];
            char_to_write++;
            message[char_to_write] = ' ';
            char_to_write++;
			
			//Enfin on ajoute l'état de la partie
            if (gagne(lettreTrouvee, strlen(motSecret))){
				message[char_to_write] = '2';
        	}
        	else if(remaining_life<0){
        		message[char_to_write] = '1';
        	}else{
        		message[char_to_write] = '0';
        	}
        	char_to_write++;
        	message[char_to_write] = '!';
        	char_to_write++;
        	message[char_to_write] = '\0';
        	char_to_write++;
        	
        	//On envoie le message
        	msg_all(sockets,message);
    }
    return NULL;
}

 int gagne(int lettreTrouvee[], long tailleMot)
  {
      long i = 0;
      int joueurGagne = 1;

      for (i = 0 ; i < tailleMot ; i++)
      {
          if (lettreTrouvee[i] == 0)
              joueurGagne = 0;
      }

      return joueurGagne;
  }

 
  int rechercheLettre(char lettre, char motSecret[], int lettreTrouvee[])
    {
        long i = 0;
        int finded = 0;

        // On parcourt motSecret pour vérifier si la lettre proposée y est
        for (i = 0 ; motSecret[i] != '\0' ; i++)
        {
            if (lettre == motSecret[i]) // Si la lettre y est
            {
                finded = 1; // On mémorise que c'était une bonne lettre
                lettreTrouvee[i] = 1; // On met à 1 la case du tableau de booléens correspondant à la lettre actuelle
            }
        }

        return finded;
    }
    

/********************************************************/
/*                                                      */
/*                      Main                            */
/*                                                      */
/********************************************************/
int main(int argc, char **argv) {
  
    int 		socket_descriptor, 		/* descripteur de socket */
			nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
			longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */

    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
			adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
    int etat=0;
    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    thread params[nb_joueurs_requis] ;
    int joueurs_prets = 0;
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		  perror("!!!! erreur : impossible de trouver le serveur a partir de son nom. !!!!");
		  exit(1);
    }
    
    /* initialisation de la structure adresse_locale avec les infos recuperees */			
    
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */

    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    /*
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
		perror("erreur : impossible de recuperer le numero de port du service desire.");
		exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    */
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    
    printf("****numero de port pour la connexion au serveur : %d ****\n", 
		   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("!!!! erreur : impossible de creer la socket de connexion avec le client. !!!!");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("!!!! erreur : impossible de lier la socket a l'adresse de connexion. !!!!");
		exit(1);
    }
    
    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,nb_joueurs_requis);

    //pioche un mot aléatoire pour le jeu
    if (!piocherMot(motSecret))
        exit(0);

    lettreTrouvee = malloc(strlen(motSecret) * sizeof(int));    
    if (lettreTrouvee == NULL)
            exit(0);

    for (int i = 0 ; i < strlen(motSecret) ; i++){
        lettreTrouvee[i] = 0;
    }    
    printf("****mot pioché !****\n");
    strcpy(motSecret,"LORIS");


    pthread_t thread_joueur;
    while((nbj_rejoins < nb_joueurs_requis))
    {
		longueur_adresse_courante = sizeof(adresse_client_courant);

		/* adresse_client_courant sera renseigné par accept via les infos du connect */
		if ((nouv_socket_descriptor = 
			accept(socket_descriptor, 
			       (sockaddr*)(&adresse_client_courant),
			       &longueur_adresse_courante))
			 < 0)
         {
			perror("!!!! erreur : impossible d'accepter la connexion avec le client. !!!!");
			exit(1);
		}
        sockets[nbj_rejoins]=nouv_socket_descriptor;
        printf("****Un nouveau joueur a rejoint la partie !****\n");
        //création des paramètres pour les joueurs
        params[nbj_rejoins] = (thread) {nouv_socket_descriptor,&etat,nbj_rejoins,&joueurs_prets}; 
        pthread_create(&thread_joueur, NULL, &pendu, &params[nbj_rejoins]);
        nbj_rejoins++;
        printf("****Nombre de joueurs : %d/%d****\n",nbj_rejoins,nb_joueurs_requis);
    }

    while(joueurs_prets != nb_joueurs_requis);  
    printf("****début du jeu**** \n");
    etat=1;
    while((jeuEnCours));

    for(int i=0; i< nbj_rejoins;i++){
        close(sockets[i]);
    }
	return 0;
}
