/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

//Fonction écoute
//Le client reste à l'écoute du serveur
static void* ecoute (void * socket_descriptor){
    int* socket = (int *) socket_descriptor;
    int jeuEnCours = 1;
    while(jeuEnCours){
    	char buffer[256];
        if(read(*socket, buffer, (int)sizeof(buffer))<0){
        	//le serveur n'existe probablement plus
        	break;
        }
        //On décode le message
        char secret_word[256];
        char life[256];
        char state[256];
        //On décrypte le message recu afin d'en extraire les 3 informations. Structure du message : "mot_codé vie etat"
        
		char delim[] = " ";

		char *ptr = strtok(buffer, delim);
		strcpy(secret_word,ptr);
		
		ptr = strtok(NULL, delim);
		strcpy(life,ptr);
		
		ptr = strtok(NULL, delim);
		strcpy(state,ptr);
        
        //On affiche les informations :
        printf("\e[1;1H\e[2J"); //clear
        printf("\nUn joueur à fait une nouvelle proposition !\n");
        printf("Il ne reste plus que %s coup(s) à jouer \n",life);
        printf("Mot mystère : %s \n", secret_word);
        
        if(state[0]=='1'){
        	printf("Perdu :C Le mot secret était %s\n",secret_word);
        	jeuEnCours = 0;
        	close(*socket);
		    exit(0);
        }
        if(state[0]=='2'){
        	printf("Gagné ! Le mot secret était %s\n",secret_word);
        	jeuEnCours = 0;
		    close(*socket);
		    exit(0);
        }
        	
    }
	return NULL;
}

/********************************************************/
/*                                                      */
/*                      Main                            */
/*                                                      */
/********************************************************/

int main(int argc, char **argv) {
  
    int 	socket_descriptor ; 	/* descripteur de socket */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    char *	prog; 			/* nom du programme */
    char *	host; 			/* nom de la machine distante */
    char * mesg; 			/* message envoyé */
    pthread_t thread_listen  ;
    char pseudo[32];
    
     
    if (argc != 3) {
		perror("usage : client <adresse-serveur> <type_message>");
		exit(1);
    }
   
    prog = argv[0];
    host = argv[1];
    mesg = argv[2];
    printf("\e[1;1H\e[2J"); //clear
    printf("nom de l'executable : %s \n", prog);
    printf("adresse du serveur  : %s \n", host);
    printf("message envoye      : %s \n", mesg);
    
    if ((ptr_host = gethostbyname(host)) == NULL) {
	perror("erreur : impossible de trouver le serveur a partir de son adresse.");
	exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
    
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
    
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le serveur.");
		exit(1);
    }
    
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
    }
    
    printf("connexion etablie avec le serveur. \n");
    printf("\e[1;1H\e[2J"); //clear
    printf("**********************************\n");
    printf("* Bienvenue dans le Jeu du Pendu!*\n");
    printf("**********************************\n");

    //Saisie du pseudo du joueur
    printf("Veuillez saisir votre pseudo : \n");
    fgets(pseudo, sizeof pseudo, stdin);
    pseudo[strcspn(pseudo, "\n")] = '\0';
    if ((write(socket_descriptor,pseudo,strlen(pseudo)))< 0){
        perror("erreur : impossible d'écrire le message destine au serveur.");
        exit(1);
    }
    
    printf("Vous avez choisi le pseudo : %s !\n", pseudo);

    // Le client se met en maintenant en écoute
    pthread_create(&thread_listen,NULL, ecoute, &socket_descriptor);

    //Tant que les messages émis sont différents de "quit" et que la partie n'est pas finie
    while((strcmp(mesg,"quit")!='\0')){
		//Envoie d'un message au serveur, seul la première lettre compte
		fgets(mesg, sizeof mesg, stdin);
		mesg[strcspn(mesg, "\n")] = '\0';
		if ((write(socket_descriptor,&mesg[0],1))< 0){
		    perror("erreur : impossible d'écrire le message au serveur.");
		    exit(1);
		}
    }
    printf("Vous quittez le Jeu du Pendu... Fermeture..\n");
    printf("\nfin de la reception.\n");
    close(socket_descriptor);
    printf("connexion avec le serveur fermee, fin du programme.\n");
    exit(0);
    
}
