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

    // On prépare un buffer pour le serveur
    static char buffer[BUFFER_LEN + 1];

    // Variable pour savoir si la connexion s'est fermée
    int isClosed;
    int len;
    // Buffer pour le choix utilisateur
    char choice[BUFFER_LEN + 1];
    // Buffer pour le nom donné par l'utilisateur
    char name[BUFFER_LEN + 1];
    // Buffer pour le prénom donné par l'utilisateur
    char surname[BUFFER_LEN + 1];
    // Buffer pour le numéro de dossier donné par l'utilisateur
    char folderNb[BUFFER_LEN + 1];
    // Buffer pour le numéro de place donné par l'utilisateur
    char placeNb[BUFFER_LEN + 1];
    // Pointeur pour la réponse à envoyer au serveur
    char *response;

    msleep(10);
    // Boucle d'éxécution
    while ((isClosed = manageServer(clientSocket)) == 0)
    {
        // On affiche le menu
        printf("[1] 🎟️ Voir le nombre de places disponibles\n");
        printf("\n");
        printf("[2] 📄 Liste des places\n");
        printf("\n");
        printf("[3] 🔐 Réserver une place\n");
        printf("\n");
        printf("[4] ❌ Annuler une réservation\n");
        printf("\n");
        printf("[5] 🚪 Exit\n");
        // On attend une entrée utilisateur
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
            case 1: // Nombre de places disponibles
                // On envoie la commande
                send(clientSocket, SEE_PLACES_IN_WORD, strlen(SEE_PLACES_IN_WORD), MSG_DONTWAIT);
                // On attend la réponse du serveur
                manageServer(clientSocket);
                break;
            case 2: // Liste des places disponibles
                // On envoie la commande
                send(clientSocket, SEE_TAKEN_PLACES_IN_WORD, strlen(SEE_TAKEN_PLACES_IN_WORD), MSG_DONTWAIT);
                // On attend la réponse du serveur
                manageServer(clientSocket);
                break;
            case 3: // Création de dossier
                printf("Veuillez renseignez votre nom ainsi que votre prenom\n");
                printf("Nom : ");
                // On lit l'entrée utilisateur
                readMessage(name);
                printf("Prenom : ");
                // On lit l'entrée utilisateur
                readMessage(surname);
                printf("Numéro de la place (optionnel) : ");
                // On lit l'entrée utilisateur
                readMessage(placeNb);

                // On prépare la réponse
                response = malloc(sizeof(char));
                strcpy(response, NEW_PLACE_IN_WORD);
                strcat(response, "_");
                strcat(response, name);
                strcat(response, "_");
                strcat(response, surname);

                // Si la place a été précisée
                if (placeNb[0] != '\n')
                {
                    // On vérifie que c'est un entier
                    for (int i = 0; i < strlen(placeNb); i++)
                    {
                        if (placeNb[i] < 48 || placeNb[i] > 57)
                        {
                            isStringError = 1;
                            break;
                        }
                    }
                    if (!isStringError)
                    {
                        sprintf(response, "%s_%d", response, atoi(placeNb) - 1);
                    }
                }
                // On envoie la demande de création
                send(clientSocket, response, strlen(response), MSG_DONTWAIT);
                free(response);
                break;
            case 4: // Annulation de dossier
                printf("Veuillez renseigner votre nom ainsi que votre numero de dossier : \n");
                printf("Nom : ");
                // On lit l'entrée utilisateur
                readMessage(name);
                printf("Numero de dossier : ");
                // On lit l'entrée utilisateur
                readMessage(folderNb);

                // On prépare la réponse
                response = malloc(sizeof(char));
                strcpy(response, CANCEL_IN_WORD);
                strcat(response, "_");
                strcat(response, name);
                strcat(response, "_");
                strcat(response, folderNb);
                // On envoie la demande d'annulation
                send(clientSocket, response, strlen(response), MSG_DONTWAIT);
                free(response);

                break;
            case 5: // Quitter
                // On envoie l'information de déconnexion
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
        // En fonction de la réponse du serveur, on affiche différemment les informations...
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
            if (strlen(buffer) > 6) // Si la réponse est plutot longue, c'est un numéro de dossier
            {
                printf("Numéro de dossier: %s", buffer);
            }
            else // Sinon c'est sûrement le nombre de place disponible
            {
                printf("Nombre de places disponibles: %s", buffer);
            }
        }
        else if (buffer[0] == '-')
        {
            printf("Liste des places :\n");
            // On initialise un tableau d'entier
            int *takenPlaces = calloc(NB_MAX_SALLE, sizeof(int));
            char *prefix = strtok(buffer, "_");
            for (int i = 0; i < NB_MAX_SALLE; i++)
            {
                // Pour chaque élément de la réponse
                char *place = strtok(NULL, "_");
                // On vérifie si on ne va pas trop loin
                if (place == NULL)
                {
                    break;
                }
                // On note que la place indiquée par le serveur est prise
                int index = atoi(place);
                takenPlaces[index] = 1;
            }
            // Pour toute les places
            for (int i = 0; i < NB_MAX_SALLE; i++)
            {
                // Si la place est prise, on note un X
                if (takenPlaces[i])
                {
                    printf("%3c ", 'X');
                }
                // Sinon on affiche le numéro de place
                else
                {
                    printf("%3d ", i + 1);
                }
                
                // Et on met 10 places par ligne
                if ((i + 1) % 10 == 0)
                {
                    printf("\n");
                }
            }

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
