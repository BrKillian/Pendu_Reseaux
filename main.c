/*
Jeu du pendu
Par M@teo21, pour le Site du Z�r0
www.siteduzero.com

main.c
------

Fonctions principales de gestion du jeu
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "dico.c"
#include "dico.h"


int gagne(int lettreTrouvee[], long tailleMot);
int rechercheLettre(char lettre, char motSecret[], int lettreTrouvee[]);
char lireCaractere();

int main(int argc, char* argv[])
{
    char lettre = 0; // Stocke la lettre propos�e par l'utilisateur (retour du scanf)
    char motSecret[100] = {0}; // Ce sera le mot � trouver
    int *lettreTrouvee = NULL; // Un tableau de bool�ens. Chaque case correspond � une lettre du mot secret. 0 = lettre non trouv�e, 1 = lettre trouv�e
    long coupsRestants = 10; // Compteur de coups restants (0 = mort)
    long i = 0; // Une petite variable pour parcourir les tableaux
    long tailleMot = 0;

    printf("Bienvenue dans le Pendu !\n\n");

    if (!piocherMot(motSecret))
        exit(0);

    tailleMot = strlen(motSecret);

    lettreTrouvee = malloc(tailleMot * sizeof(int)); // On alloue dynamiquement le tableau lettreTrouvee (dont on ne connaissait pas la taille au d�part)
    if (lettreTrouvee == NULL)
        exit(0);

    for (i = 0 ; i < tailleMot ; i++)
        lettreTrouvee[i] = 0;

    // On continue � jouer tant qu'il reste au moins un coup � jouer ou qu'on
    // n'a pas gagn�
    while (coupsRestants > 0 && !gagne(lettreTrouvee, tailleMot))
    {
        printf("\n\nIl vous reste %ld coups a jouer", coupsRestants);
        printf("\nQuel est le mot secret ? ");

        /* On affiche le mot secret en masquant les lettres non trouv�es
        Exemple : *A**ON */
        for (i = 0 ; i < tailleMot ; i++)
        {
            if (lettreTrouvee[i]) // Si on a trouv� la lettre n�i
                printf("%c", motSecret[i]); // On l'affiche
            else
                printf("*"); // Sinon, on affiche une �toile pour les lettres non trouv�es
        }

        printf("\nProposez une lettre : ");
        lettre = lireCaractere();

        // Si ce n'�tait PAS la bonne lettre
        if (!rechercheLettre(lettre, motSecret, lettreTrouvee))
        {
            coupsRestants--; // On enl�ve un coup au joueur
        }
    }


    if (gagne(lettreTrouvee, tailleMot))
        printf("\n\nGagne ! Le mot secret etait bien : %s", motSecret);
    else
        printf("\n\nPerdu ! Le mot secret etait : %s", motSecret);

    free(lettreTrouvee); // On lib�re la m�moire allou�e manuellement (par malloc)

        return 0;
}


char lireCaractere()
{
    char caractere = 0;

    caractere = getchar(); // On lit le premier caract�re
    caractere = toupper(caractere); // On met la lettre en majuscule si elle ne l'est pas d�j�

    // On lit les autres caract�res m�moris�s un � un jusqu'� l'\n
    while (getchar() != '\n') ;

    return caractere; // On retourne le premier caract�re qu'on a lu
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

    // On parcourt motSecret pour v�rifier si la lettre propos�e y est
    for (i = 0 ; motSecret[i] != '\0' ; i++)
    {
        if (lettre == motSecret[i]) // Si la lettre y est
        {
            bonneLettre = 1; // On m�morise que c'�tait une bonne lettre
            lettreTrouvee[i] = 1; // On met � 1 le case du tableau de bool�ens correspondant � la lettre actuelle
        }
    }

    return bonneLettre;
}
