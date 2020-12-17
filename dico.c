
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "dico.h"

//piocher un mot depuis le fichier dico.txt
int piocherMot(char *motPioche)
{
    FILE* dico = NULL; // Le pointeur de fichier qui va contenir notre fichier
    int nombreMots = 0, numMotChoisi = 0;
    int caractereLu = 0;
    dico = fopen("dico.txt", "r"); // On ouvre le dictionnaire en lecture seule

    // On vérifie si on a réussi à ouvrir le dictionnaire
    if (dico == NULL) // Si on n'a PAS réussi à ouvrir le fichier
    {
        printf("\nImpossible de charger le dictionnaire de mots");
        return 0; // On retourne 0 pour indiquer que la fonction a �chou�
        // A la lecture du return, la fonction s'arr�te imm�diatement.
    }

    // On compte le nombre de mots dans le fichier (il suffit de compter les
    // entrees \n
    do
    {
        caractereLu = fgetc(dico);
        if (caractereLu == '\n')
            nombreMots++;
    } while(caractereLu != EOF);

    numMotChoisi = nombreAleatoire(nombreMots); // On pioche un mot au hasard

    // On recommence a lire le fichier depuis le debut. On s'arrete lorsqu'on est arrives au bon mot
    rewind(dico);
    while (numMotChoisi > 0)
    {
        caractereLu = fgetc(dico);
        if (caractereLu == '\n')
            numMotChoisi--;
    }

    /* Le curseur du fichier est positionne au bon endroit.
    On n'a plus qu'a faire un fgets qui lira la ligne */
    fgets(motPioche, 100, dico);

    // On vire l'\n � la fin
    motPioche[strlen(motPioche) - 1] = '\0';
    fclose(dico);

    return 1; // Tout s'est bien passe, on retourne 1
}

int nombreAleatoire(int nombreMax)
{
    srand(time(NULL));
    return (rand() % nombreMax);
}


