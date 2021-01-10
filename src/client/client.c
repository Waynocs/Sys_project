/**
 * @ Author: SUBLET Tom & SERANO Waïan
 * @ Create Time: 2021-01-02 00:30:36
 * @ Description: The client
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "../common/sockets.h"
#include "../common/sleep.h"
#include "../common/words.h"
#include "client.h"

/*
 * Lis l'entrée utilisateur et la copie dans un buffer
 * 
 * @param tampon Le buffer dans lequel copier l'entrée
*/
void readMessage(char tampon[])
{
    //printf("Saisir un message à envoyer :\n");
    fgets(tampon, BUFFER_LEN, stdin);
    strtok(tampon, "\n");
}

int main(int argc, char const *argv[])
{
    // Structure contenant l'adresse
    struct sockaddr_in adresse;
    initAdresse(&adresse);
    // Descripteur de la socket du serveur
    int clientSocket = initClientSocket(&adresse);

    static char buffer[BUFFER_LEN + 1];

    int isClosed;
    int len;
    char choice[BUFFER_LEN + 1];
    char name[BUFFER_LEN + 1];
    char surname[BUFFER_LEN + 1];
    char folderNb[BUFFER_LEN + 1];
    char placeNb[BUFFER_LEN + 1];
    char *response;

    msleep(10);
    // Boucle d'éxécution
    while ((isClosed = manageServer(clientSocket)) == 0)
    {
        printf("--1-- Voir les places disponibles\n--2-- liste des places\n--3-- Réserver une place\n--4-- Annuler une réservation\n--5-- Exit\n");
        readMessage(choice);
        int isStringError = 0;
        // On vérifie si le paramètre est bien un entier
        for (int i = 0; i < strlen(choice); i++)
        {
            if (choice[i] < 48 || choice[i] > 57)
            {
                isStringError = 1;
                break;
            }
        }
        if (!isStringError)
        {
            // On transforme le paramètre en entier
            int index = atoi(choice);
            switch (index)
            {
            case 1:
                send(clientSocket, SEE_PLACES_IN_WORD, strlen(SEE_PLACES_IN_WORD), MSG_DONTWAIT);
                manageServer(clientSocket);
                break;
            case 2:
                send(clientSocket, SEE_TAKEN_PLACES_IN_WORD, strlen(SEE_TAKEN_PLACES_IN_WORD), MSG_DONTWAIT);
                manageServer(clientSocket);
                break;
            case 3:
                printf("Veuillez renseignez votre nom ainsi que votre prenom\n");
                printf("Nom : ");
                readMessage(name);
                printf("Prenom : ");
                readMessage(surname);
                printf("Numéro de la place (optionnel) : ");
                readMessage(placeNb);

                response = malloc(sizeof(char));
                strcpy(response, NEW_PLACE_IN_WORD);
                strcat(response, "_");
                strcat(response, name);
                strcat(response, "_");
                strcat(response, surname);

                if (placeNb[0] != '\n')
                {
                    strcat(response, "_");
                    strcat(response, placeNb);
                }
                send(clientSocket, response, strlen(response), MSG_DONTWAIT);
                free(response);

                break;
            case 4:
                printf("Veuillez renseigner votre nom ainsi que votre numero de dossier : \n");
                printf("Nom : ");
                readMessage(name);
                printf("Numero de dossier : ");
                readMessage(folderNb);

                response = malloc(sizeof(char));
                strcpy(response, CANCEL_IN_WORD);
                strcat(response, "_");
                strcat(response, name);
                strcat(response, "_");
                strcat(response, folderNb);

                send(clientSocket, response, strlen(response), MSG_DONTWAIT);
                free(response);

                break;
            case 5:
                send(clientSocket, EXIT_IN_WORD, strlen(EXIT_IN_WORD), MSG_DONTWAIT);
                manageServer(clientSocket);
                return EXIT_SUCCESS;
            }
        }
        printf("\n");
        msleep(10);
    }
    return EXIT_SUCCESS;
}

// Initialisation de la structure sockaddr_in
void initAdresse(struct sockaddr_in *adresse)
{
    (*adresse).sin_family = AF_INET;
    inet_aton("127.0.0.1", &adresse->sin_addr);
    (*adresse).sin_port = htons(PORT);
}

// Démarrage de la socket client
int initClientSocket(struct sockaddr_in *adresse)
{
    int fdsocket = initSocket(adresse);

    // Connexion de la socket sur le port et l'adresse IP
    printf("Connexion de la socket sur le port %i\n", PORT);
    if (connect(fdsocket, (struct sockaddr *)adresse, sizeof(*adresse)) != 0)
    {
        printf("Echec de connexion: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fdsocket;
}

// Lecture de la réponse
int manageServer(int clientSocket)
{
    static char buffer[BUFFER_LEN + 1];

    int len = recv(clientSocket, buffer, BUFFER_LEN, MSG_DONTWAIT);
    // Booléen pour suivre l'état de la socket
    int isClosed = 0;
    if (len == -1 && errno != EAGAIN)
    {
        // Une erreur est survenue
        printf("Errno [%i] : %s\n", errno, strerror(errno));
        isClosed = 1;
    }
    else if (len == 0)
    {
        // Le serveur s'est déconnecté (extrémité de la socket fermée)
        isClosed = 1;
    }
    else if (len > 0)
    {
        // Clear la commande
        system("clear");
        if (strcmp(buffer, SUCCESS_OUT_WORD) == 0)
        {
            printf("Action réussie.");
        }
        else if (strcmp(buffer, TAKEN_ERROR_OUT_WORD) == 0)
        {
            printf("La place demandée est déjà reservée.");
        }
        else if (strcmp(buffer, FNAME_ERROR_OUT_WORD) == 0)
        {
            printf("Le prénom donné est null.");
        }
        else if (strcmp(buffer, NODOSS_ERROR_OUT_WORD) == 0)
        {
            printf("Le numéro de dossier donné est null.");
        }
        else if (strcmp(buffer, EXIST_ERROR_OUT_WORD) == 0)
        {
            printf("Le dossier n'existe pas.");
        }
        else if (strcmp(buffer, NO_CMD_ERROR_OUT_WORD) == 0)
        {
            printf("La comande n'a pas été comprise.");
        }
        else if (strcmp(buffer, EXIT_OUT_WORD) == 0)
        {
            printf(EXIT_OUT_WORD);
        }
        else if (strcmp(buffer, READY_OUT_WORD) == 0)
        {
            printf(READY_OUT_WORD);
        }
        else if (buffer[0] >= 48 && buffer[0] <= 57)
        {
            if (strlen(buffer) > 6)
            {
                printf("Numéro de dossier: %s", buffer);
            }
            else
            {
                printf("Nombre de places disponibles: %s", buffer);
            }
        }
        else if (buffer[0] == '-')
        {
            printf("Liste des places :\n");
            int *takenPlaces = calloc(0, NB_MAX_SALLE * sizeof(int));
            char *prefix = strtok(buffer, "_");
            for (int i = 0; i < NB_MAX_SALLE; i++)
            {
                char *place = strtok(NULL, "_");
                printf("-> %s", place);
                if (place == NULL)
                {
                    break;
                }
                int index = atoi(place);
                takenPlaces[index] = 1;
            }
            for (int i = 0; i < NB_MAX_SALLE; i++)
            {
                if (takenPlaces[i])
                {
                    printf("X ");
                }
                else
                {
                    printf("%d ", i + 1);
                }
            }

            printf("\n");
            free(takenPlaces);
        }
        printf("\n");
        memset(buffer, 0, sizeof(buffer));
    }

    if (isClosed == 1)
    {
        // La socket est fermé ou le client veut quitter le serveur !
        printf("Fermeture de la connexion avec le serveur\n");
        // Fermeture de la socket
        close(clientSocket);
    }
    return isClosed;
}