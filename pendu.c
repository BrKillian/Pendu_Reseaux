#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Declaration des structures

//Structure Joueur : contenant les informations des joueurs (pseudo, ordre).

//Structure propostion :Structure regroupant le mot crypté, et les lettres proposées.

//Structure pendu : Structure ayant le mot, le nombre de tentative et le nombre de vies restantes.


main()
{
    printf("ça marche \n");
    return 0;
}


/*InitMot : Elle génère un mot du dictionnaire Larousse*/
void InitMot()
{}

/*InitPendu : Initialise la structure Pendu*/
void Initpendu()
{}

/*InitJoueur : Permet de renseigner le nombre de joueurs et initialise la structure joueur pour chacun des joueurs.*/
void InitJoueur()
{}

/*GestionMot :  Fonction qui va faire deux choses distinctes, 
renseigner dans la liste des lettres proposées la proposition du joueur et si la lettre proposée fait partie du mot à trouver,
la fonction la renseigne dans le mot crypté.*/
void GestionMot()
{}

/*Jeu : Tant que le mot n’est pas trouvé OU le nombre de proposition maximum autorisé n’est pas atteint : 
Elle affiche le mot crypté , la liste des lettres proposées, demande au joueur une lettre , 
vérifie la saisie, appelle la fonction gestionMot. Pour chacun des joueurs.*/
void Jeu()
{}

/*Affich_Crypte : Affichage du mot crypté.*/
void Affich_Crypte()
{}

/*Affich_Lettres : Affichage des lettres proposées .*/
void Affich_Lettres()
{}

/*Affich_Vie : Affichage du nombre d’essai restant sur le nombre d’essai maximal. */
void Affich_Vie()
{}

/*Verification_Saisie : à voir si il est préférable de créer une fonction a part ou de la contenir dans la fonction principale “Jeu”.*/
bool Verif_Saisie()
{
    return true;
}