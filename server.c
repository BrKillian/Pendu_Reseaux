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
#include <string.h> 		/* pour bcopy, ... */  

#define TAILLE_MAX_NOM 256
#define NB_JOUEURS 5

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;




/*------------------------------------------------------*/


//DECLARATION DE FONCTION
int gagne(int lettreTrouvee[], long tailleMot);
int rechercheLettre(char lettre, char motSecret[], int lettreTrouvee[]);
char lireCaractere();
void pendu();
void message_global(int socks[], char * message);


void msg_all(int socks[5],char * msg){
  for(int i =0;i< NB_JOUEURS ; i++){
    write(socks[i],msg,strlen(msg));
  }
}

//JEU DU PENDU

void pendu()
{
    char lettre = 0; // Stocke la lettre proposée par l'utilisateur (retour du scanf)
    char motSecret[100] = {0}; // Ce sera le mot à trouver
    int *lettreTrouvee = NULL; // Un tableau de booléens. Chaque case correspond à une lettre du mot secret. 0 = lettre non trouvée, 1 = lettre trouvée
    long coupsRestants = 10; // Compteur de coups restants (0 = mort)
    long i = 0; // Une petite variable pour parcourir les tableaux
    long tailleMot = 0;

    printf("Bienvenue dans le Pendu !\n\n");

    if (!piocherMot(motSecret))
        exit(0);

    tailleMot = strlen(motSecret);
    lettreTrouvee = malloc(tailleMot * sizeof(int)); // On alloue dynamiquement le tableau lettreTrouvee (dont on ne connaissait pas la taille au départ)
    
    if (lettreTrouvee == NULL)
        exit(0);

    for (i = 0 ; i < tailleMot ; i++)
        lettreTrouvee[i] = 0;

    /* On continue à jouer tant qu'il reste au moins un coup à jouer ou qu'on
     n'a pas gagné */
    while (coupsRestants > 0 && !gagne(lettreTrouvee, tailleMot))
    {
        printf("\n\nIl vous reste %ld coups a jouer", coupsRestants);
        printf("\nQuel est le mot secret ? ");

        /* On affiche le mot secret en masquant les lettres non trouvées
        Exemple : *A**ON */
        for (i = 0 ; i < tailleMot ; i++)
        {
            if (lettreTrouvee[i]) // Si on a trouvé la lettre n° i
                printf("%c", motSecret[i]); // On l'affiche
            else
                printf("*"); // Sinon, on affiche une étoile pour les lettres non trouvées
        }

        printf("\nProposez une lettre : ");
        lettre = lireCaractere();

        // Si ce n'était PAS la bonne lettre
        if (!rechercheLettre(lettre, motSecret, lettreTrouvee))
        {
            coupsRestants--; // On enlève un coup au joueur
        }
    }

    if (gagne(lettreTrouvee, tailleMot))
        printf("\n\nGagne ! Le mot secret etait bien : %s", motSecret);
    else
        printf("\n\nPerdu ! Le mot secret etait : %s", motSecret);

    free(lettreTrouvee); // On libère la mémoire allouée manuellement (par malloc)
 
}

char lireCaractere()
{
    char caractere = 0;

    caractere = getchar(); // On lit le premier caractère
    caractere = toupper(caractere); // On met la lettre en majuscule si elle ne l'est pas déjà

    // On lit les autres caractères mémorisés un à un jusqu'au \n
    while (getchar() != '\n') ;

    return caractere; // On retourne le premier caractère qu'on a lu
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
    



/*void renvoi (int sock) {

    char buffer[256];
    int longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0) 
    	return;
    
    printf("message lu : %s \n", buffer);
    
    buffer[0] = 'R';
    buffer[1] = 'E';
    buffer[longueur] = '#';
    buffer[longueur+1] ='\0';
    
    printf("message apres traitement : %s \n", buffer);
    
    printf("renvoi du message traite.\n");

    // mise en attente du prgramme pour simuler un delai de transmission 
    sleep(3);
    
    write(sock,buffer,strlen(buffer)+1);
    
    printf("message envoye. \n");
        
    return;
    
}*/


/*------------------------------------------------------*/

/*------------------------------------------------------*/
main(int argc, char **argv) {
  
    int 		socket_descriptor, 		/* descripteur de socket */
			nouv_socket_descriptor[NB_JOUEURS], 	/* [nouveau] descripteur de socket */
			longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */

    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
			adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 			/* les infos recuperees sur le service de la machine */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
    pthread_t thread_joueurs[NB_JOUEURS]; //Tableau contenant les threads des joueurs 
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
    listen(socket_descriptor,NB_JOUEURS);

    /* Création des différents thread  */
    for(int i = 0; i < NB_JOUEURS; ++i)  {
    
		  longueur_adresse_courante = sizeof(adresse_client_courant);
		

		  /* adresse_client_courant sera renseigné par accept via les infos du connect */
		  if ((nouv_socket_descriptor[i] = accept(socket_descriptor, (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante))< 0) {
			  perror("erreur : impossible d'accepter la connexion avec le client.");
			  exit(1);
		  }

  
      if( pthread_create( &thread_joueurs[i] , NULL ,  pendu , (void*) &nouv_socket_descriptor[i]) < 0)
      {
        perror("erreur : impossible de créer le thread");
        exit(1);
      }
      
    
		  /* traitement du message */
		  printf("reception d'un message.\n");
					
		  close(nouv_socket_descriptor[i]);
	
	
    }
    
}
