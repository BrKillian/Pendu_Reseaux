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
    int * socket; // le socket géré 
    int * etat; /* Variable état partie 0 -début 1 encours 2 fin*/
    char * pseudo[32]; // pointeur vers pseudo
    int * numJoueur; // pointeur vers num du joueur
}typedef thread;


//VARIABLE GLOBALE
int nb_joueurs = 2 ;
int nbj = 0;
int sockets[2];
int 	socket_descriptor ; 	/* descripteur de socket */
int 	longueur; 		/* longueur d'un buffer utilisé */
char 	buffer[256];
int motDejaPiocher = 0; // boolean 0:false, 1:true
char motSecret[100] = {0}; // Ce sera le mot à trouver
long tailleMot = 0;
int *lettreTrouvee = NULL; // Un tableau de booléens. Chaque case correspond à une lettre du mot secret. 0 = lettre non trouvée, 1 = lettre trouvée
int finJeu = 0;
long coupsRestants = 10; // Compteur de coups restants (0 = mort)

//DECLARATION DE FONCTION
int gagne(int lettreTrouvee[], long tailleMot);
int rechercheLettre(char lettre, char motSecret[], int lettreTrouvee[]);
char lireCaractere(char  caractere);
void pendu(struct Sthread * param);
void msg_all(int socks[], char * message);

void msg_all(int socks[5],char * msg){
  for(int i =0;i< nb_joueurs ; i++){
    write(socks[i],msg,strlen(msg));
  }
}

void pendu(struct Sthread * param)
{
    char lettre = 0; // Stocke la lettre proposée par l'utilisateur (retour du scanf)
    long i = 0; // variable pour parcourir les tableaux

    printf("Un nouveau joueur a rejoint la partie !\n\n");
    
    // pour le premier joueur on pioche un mot puis on alloue et init le tab lettre_trouvee
    if(motDejaPiocher == 0){
        if (!piocherMot(motSecret))
        exit(0);

        tailleMot = strlen(motSecret);
        lettreTrouvee = malloc(tailleMot * sizeof(int));    
        if (lettreTrouvee == NULL)
            exit(0);

        for (i = 0 ; i < tailleMot ; i++){
            lettreTrouvee[i] = 0;
        }    
        motDejaPiocher = 1;
        printf("mot piocher !\n\n");
    }
    
    struct Sthread * params = param;
    printf("numéro joueur courant %d\n\n",(* params).numJoueur);
    printf("etat du joueur courant %d\n\n",(* params).etat);
     //début du programme 
    while((* params).etat == 0){
        read((* params).socket,buffer,sizeof(buffer));
        strcpy((* params).pseudo,&buffer);

        strcpy(buffer,("Pseudo du joueur : %s\n",(* params).pseudo));
        write((* params).socket ,buffer,strlen(buffer)+1);
       
        if ( (* params).numJoueur== nb_joueurs){
            (* params).etat = 1;
        }
    }
         printf("après création pseudo\n"); // DEBUG

    while((* params).etat == 1){
        // On continue à jouer tant qu'il reste au moins un coup à jouer ou qu'on à pas gagner
        while (coupsRestants > 0 && !gagne(lettreTrouvee, tailleMot))
        {

            strcpy(buffer,("\n\nIl vous reste %ld coups a jouer \n", coupsRestants));
            write((* params).socket ,buffer,strlen(buffer)+1);

            strcpy(buffer,("\nQuel est le mot secret ? \n"));
            write((* params).socket ,buffer,strlen(buffer)+1);
            

            // On affiche le mot secret en masquant les lettres non trouvées 
            for (i = 0 ; i < tailleMot ; i++)
            {
                // Si on a trouvé la lettre n° i
                if (lettreTrouvee[i]) {
                    strcpy(buffer,("%c", motSecret[i]));
                    write((* params).socket ,buffer,strlen(buffer)+1);
                }
                else{
                    strcpy(buffer,"*");
                    write((* params).socket ,buffer,strlen(buffer)+1);
                }
                    
            }  
            strcpy(buffer,"\nProposez une lettre : \n");
            write((* params).socket ,buffer,strlen(buffer)+1);

            listen((* params).socket,1);

            lettre = read((* params).socket, buffer, 1);
            strcpy(buffer,("message lu : %s \n", buffer));
            write((* params).socket ,buffer,strlen(buffer)+1);
            lettre = lireCaractere(lettre);

            // Si ce n'était PAS la bonne lettre
            if (!rechercheLettre(lettre, motSecret, lettreTrouvee))
            {
                coupsRestants--; 
            }

            if (gagne(lettreTrouvee, tailleMot)){
                strcpy(buffer,("\n\nGagne ! Le mot secret etait bien : %s \n", motSecret));
                msg_all(sockets,buffer);
                finJeu = 1;
                //libération mémoire du tableau alloué
                free(lettreTrouvee);
            }
            else{
                strcpy(buffer,("\n\nPerdu ! Le mot secret etait : %s \n", motSecret));
                msg_all(sockets,buffer);
                finJeu = 1;
                //libération mémoire du tableau alloué
                 free(lettreTrouvee);
            } 
        }
    }
    printf("après fin jeu \n"); // DEBUG
    
}

char lireCaractere(char caractere)
{
    //On met la lettre en majuscule si elle ne l'est pas déjà
        caractere = toupper(caractere); 
    return caractere; 
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
        int bonneLettre = 0;

        // On parcourt motSecret pour vérifier si la lettre proposée y est
        for (i = 0 ; motSecret[i] != '\0' ; i++)
        {
            if (lettre == motSecret[i]) // Si la lettre y est
            {
                bonneLettre = 1; // On mémorise que c'était une bonne lettre
                lettreTrouvee[i] = 1; // On met à 1 la case du tableau de booléens correspondant à la lettre actuelle
            }
        }

        return bonneLettre;
    }
    

/*------------------------------------------------------*/
main(int argc, char **argv) {
  
    int 		socket_descriptor, 		/* descripteur de socket */
			nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
			longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */

    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
			adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 			/* les infos recuperees sur le service de la machine */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
    pthread_t threads[nb_joueurs];  // tableau des threads
    int ret ;/* Retour Thread*/
    
    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		  perror("erreur : impossible de trouver le serveur a partir de son nom.");
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
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }
    
    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,nb_joueurs);

    pthread_t thread_joueur;
    printf("ça passe ici : avant while \n");
    while((nbj < nb_joueurs ) || (finJeu == 0))
    {
        printf("re modif: debut while \n");
    
		longueur_adresse_courante = sizeof(adresse_client_courant);

		/* adresse_client_courant sera renseigné par accept via les infos du connect */
		if ((nouv_socket_descriptor = 
			accept(socket_descriptor, 
			       (sockaddr*)(&adresse_client_courant),
			       &longueur_adresse_courante))
			 < 0)
         {
			perror("erreur : impossible d'accepter la connexion avec le client.");
			exit(1);
		}
        sockets[nbj]=nouv_socket_descriptor;
        nbj++;
        
        printf("Nombre de joueurs : %d \n",nbj);
        printf("Socket rejoint : %s \n", sockets[nbj]);

        //création des paramètres pour les joueurs
        struct Sthread params = {&sockets[nbj],0," ",nbj}; 
        printf("création params\n"); // DEBUG

        if(threads[nbj] = pthread_create(&thread_joueur,NULL,pendu, &params) < 0){
            perror("erreur : impossible d'accepter la connexion avec le client. \n");
           exit(1);
        }
         
    }

    for(int i=0; i< nb_joueurs;i++){
        close(sockets[i]);
    }

}
