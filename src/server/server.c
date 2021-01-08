/**
 * @ Author: SUBLET Tom & SERANO Waïan
 * @ Create Time: 2021-01-02 00:30:36
 * @ Description: The server
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
#include <signal.h>
#include <threads.h>

#include "../common/sockets.h"
#include "../common/words.h"
#include "server.h"

struct Salle salle;
struct Client clients[NB_CLIENTS];
int isRunning = 1;

void exitHandler(int dummy)
{
    //TODO: double free or corruption (!prev)
    isRunning = 0;
    for (int i = 0; i < NB_MAX_SALLE; i++)
    {
        if (salle.places[i].noDoss != NULL)
        {
            free(salle.places[i].nom);
            free(salle.places[i].prenom);
            free(salle.places[i].noDoss);
        }
    }

    free(salle.places);
}

int main(void)
{
    srand((unsigned int)time(NULL));

    for (int i = 0; i < NB_MAX_SALLE; i++)
    {
        clients[i].id = -1;
        clients[i].socket = -1;
    }

    salle.places = malloc(NB_MAX_SALLE * sizeof(struct Place));
    salle.nbNonLibres = 0;
    // Avoid memory leaks
    signal(SIGINT, exitHandler);

    // Structure contenant l'adresse
    struct sockaddr_in adresse;
    // Initialisation de l'adresse
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = IP;
    adresse.sin_port = htons(PORT);

    // Descripteur de la socket du serveur
    int serverSocket = initServerSocket(&adresse);

    int clientSocket;
    int isEmptyPlace = 0;
    while (1)
    {
        // Descripteur de la socket du client, on attend une connexion
        if ((clientSocket = waitForClient(&serverSocket)) != -1)
        {
            printf("[INFO - SERVER] Attribution de l'identifiant\n");
            for (int i = 0; i < NB_CLIENTS; i++)
            {
                if (clients[i].id == -1)
                {
                    // On créé un nouveau client
                    struct Client client;
                    client.socket = clientSocket;
                    client.id = i;
                    clients[i] = client;

                    // On attribue un thread au nouveau client
                    thrd_t thread;
                    if (thrd_create(&thread, clientMain, &(clients[i])) != thrd_success)
                    {
                        printf("[ERROR - CLIENT %d] Error thread\n", clients[i].id);
                    }

                    isEmptyPlace = 1;
                    break;
                }
            }
            if (!isEmptyPlace)
            {
                printf("[INFO - SERVER] Plus de place");
            }
        }
    }
    return EXIT_SUCCESS;
}

int clientMain(struct Client *client)
{
    printf("[INFO - CLIENT %d] Création du thread\n", client->id);
    send(client->socket, READY_OUT_WORD, strlen(READY_OUT_WORD), MSG_DONTWAIT);

    int isClosed = 0;

    while (isRunning && !isClosed)
    {
        isClosed = manageClient(client);
    }

    printf("[INFO - CLIENT %d] Fin du thread\n", client->id);
    clients[client->id].id = -1;
    return thrd_success;
}

// On traite l'input des clients
int manageClient(struct Client *client)
{
    // Création d'un tampon pour stocker les messages des clients dans la heap
    static char buffer[BUFFER_LEN + 1];

    // On récupère l'état de la socket
    int len = recv(client->socket, buffer, BUFFER_LEN, MSG_DONTWAIT);

    // Booléen pour suivre l'état de la socket
    int isClosed = 0;
    if (len == -1 && errno != EAGAIN)
    {
        // Une erreur est survenue
        printf("[ERROR - CLIENT %d] Errno {%i} : %s\n", client->id, errno, strerror(errno));
        isClosed = 1;
    }
    else if (len == 0)
    {
        // Le client s'est déconnecté (extrémité de la socket fermée)
        isClosed = 1;
    }
    else if (len > 0)
    {
        // Ajout du terminateur de chaîne
        buffer[len] = '\0';
        printf("[INFO - CLIENT %d] Commande : %s\n", client->id, buffer);
        manageCommands(client, buffer);
    }

    if (isClosed == 1)
    {
        // La socket est fermé ou le client veut quitter le serveur !
        printf("[INFO - CLIENT %d] Fermeture de la connexion\n", client->id);
        // Fermeture de la socket
        close(client->socket);
    }

    return isClosed;
}

void manageCommands(struct Client *client, char *buffer)
{
    char *response = malloc(sizeof(char));
    char *command = strtok(buffer, "_");

    if (strncmp(buffer, EXIT_IN_WORD, strlen(EXIT_IN_WORD)) == 0)
    {
        // Le client veut se déconnecter
        send(client->socket, EXIT_OUT_WORD, strlen(EXIT_OUT_WORD), 0);
        // La socket est fermé ou le client veut quitter le serveur !
        printf("[INFO - CLIENT %d] Fermeture de la connexion\n", client->id);
        // Fermeture de la socket
        close(client->socket);
        return;
    }
    else if (strncmp(command, SEE_PLACES_IN_WORD, strlen(SEE_PLACES_IN_WORD)) == 0)
    {
        sprintf(response, "%d", NB_MAX_SALLE - salle.nbNonLibres);
        printf("[INFO - CLIENT %d] Consultation des places dispos (Réponse: %s)\n", client->id, response);
    }
    else if (strncmp(command, SEE_TAKEN_PLACES_IN_WORD, strlen(SEE_TAKEN_PLACES_IN_WORD)) == 0)
    {
        char *places = malloc(sizeof(char));
        strcpy(places, "-");
        for (int i = 0; i < NB_MAX_SALLE; i++)
        {
            if (salle.places[i].noDoss != NULL)
            {
                sprintf(places, "%s_%d", places, i);
            }
        }

        int len = strlen(places);
        response = malloc(len * sizeof(char) + 2);
        strcpy(response, places);
        printf("[INFO - CLIENT %d] Consultation des places prises (Réponse: %s)\n", client->id, response);
        free(places);
    }
    else if (strncmp(command, NEW_PLACE_IN_WORD, strlen(NEW_PLACE_IN_WORD)) == 0)
    {
        struct Place place;
        int index = -1;
        int isPlaceError = 0;

        char *name = strtok(NULL, "_");        // Get the first parameter
        char *fname = strtok(NULL, "_");       // Get the second parameter
        char *chosenPlace = strtok(NULL, "_"); // Get the third parameter
        if (chosenPlace != NULL)
        {
            index = atoi(chosenPlace);
            if (salle.places[index].noDoss != NULL)
            {
                strcpy(response, TAKEN_ERROR_OUT_WORD);
                printf("[ERROR - CLIENT %d] Demande une place... La place %d est déjà prise\n", client->id, index);
                isPlaceError = 1;
            }
            else
            {
                int len = strlen("reserved");
                salle.places[index].noDoss = malloc(len * sizeof(char));
                strcpy(salle.places[index].noDoss, "reserved");
            }
        }

        if (!isPlaceError)
        {
            if (name != NULL)
            {
                int len = strlen(name);
                place.nom = malloc(len * sizeof(char));
                strcpy(place.nom, name);

                if (fname != NULL)
                {
                    len = strlen(fname);
                    place.prenom = malloc(len * sizeof(char));
                    strcpy(place.prenom, fname);

                    int isAlreadySet = 0;
                    char noDoss[10];

                    do
                    {
                        for (int i = 0; i < 10; i++)
                        {
                            noDoss[i] = (rand() % 10) + 48;
                        }
                        noDoss[10] = '\0'; // set the end of the string

                        for (int i = 0; i < NB_MAX_SALLE; i++)
                        {
                            if (salle.places[i].noDoss != NULL && strcmp(salle.places[i].noDoss, noDoss) == 0)
                            {
                                isAlreadySet = 1;
                                break;
                            }
                        }
                    } while (isAlreadySet);

                    len = strlen(noDoss);
                    place.noDoss = malloc(len * sizeof(char));
                    strcpy(place.noDoss, noDoss);

                    if (index == -1)
                    {
                        index = salle.nbNonLibres;
                    }
                    salle.places[index] = place;
                    salle.nbNonLibres++;
                    printf("[INFO - CLIENT %d] %s %s crée le dossier %s à la place %d\n", client->id, name, fname, noDoss, index);

                    strcpy(response, noDoss);
                }
                else
                {
                    strcpy(response, FNAME_ERROR_OUT_WORD);
                    printf("[ERROR - CLIENT %d] Le fName donné est null\n", client->id);
                }
            }
            else
            {
                strcpy(response, NAME_ERROR_OUT_WORD);
                printf("[ERROR - CLIENT %d] Le name donné est null\n", client->id);
            }
        }
    }
    else if (strncmp(command, CANCEL_IN_WORD, strlen(CANCEL_IN_WORD)) == 0)
    {
        char *name = strtok(NULL, "_");
        char *noDoss = strtok(NULL, "_");

        int index = -1;

        if (name != NULL)
        {
            if (noDoss != NULL)
            {
                for (int i = 0; i < NB_MAX_SALLE; i++)
                {
                    if (salle.places[i].noDoss != NULL)
                    {
                        if (strcmp(salle.places[i].noDoss, noDoss) == 0 && strcmp(salle.places[i].nom, name) == 0)
                        {
                            index = i;
                            break;
                        }
                    }
                }

                if (index > -1)
                {
                    int len = strlen(SUCCESS_OUT_WORD);
                    response = malloc(len * sizeof(char) + 2);
                    printf("[INFO - CLIENT %d] %s - %s supprimé\n", client->id, name, noDoss);
                    strcpy(response, SUCCESS_OUT_WORD);
                    salle.nbNonLibres--;
                }
                else
                {
                    strcpy(response, EXIST_ERROR_OUT_WORD);
                    printf("[ERROR - CLIENT %d] Le dossier n'existe pas\n", client->id);
                }
            }
            else
            {
                strcpy(response, NODOSS_ERROR_OUT_WORD);
                printf("[ERROR - CLIENT %d] Le noDoss donné est null\n", client->id);
            }
        }
        else
        {
            strcpy(response, NAME_ERROR_OUT_WORD);
            printf("[ERROR - CLIENT %d] Le name donné est null\n", client->id);
        }
    }
    else
    {
        int len = strlen(NO_CMD_ERROR_OUT_WORD);
        response = malloc(len * sizeof(char) + 2);
        strcpy(response, NO_CMD_ERROR_OUT_WORD);
        printf("[ERROR - CLIENT %d] Commande non comprise\n", client->id);
    }
    // Un seul envoi permet de ne pas surcharger le réseau
    strcat(response, "\0");
    send(client->socket, response, strlen(response), 0);
    free(response);
}

// Démarrage de la socket serveur
int initServerSocket(struct sockaddr_in *adresse)
{
    int fdsocket = initSocket(adresse);

    // Attachement de la socket sur le port et l'adresse IP
    printf("[INFO - SERVER] Attachement de la socket sur le port %i\n", PORT);
    if (bind(fdsocket, (struct sockaddr *)adresse, sizeof(*adresse)) != 0)
    {
        printf("[ERROR - SERVER] Echec d'attachement: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Passage en écoute de la socket
    printf("[INFO - SERVER] Mise en écoute de la socket\n");
    if (listen(fdsocket, BACKLOG) != 0)
    {
        printf("[ERROR - SERVER] Echec de la mise en écoute: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Passage en mode non bloquant
    fcntl(fdsocket, F_SETFL, O_NONBLOCK);

    return fdsocket;
}

// Attente de connexion d'un client
int waitForClient(int *serverSocket)
{
    // Descripteur de socket
    int clientSocket;
    // Structure contenant l'adresse du client
    struct sockaddr_in clientAdresse;
    int addrLen = sizeof(clientAdresse);

    if ((clientSocket = accept(*serverSocket, (struct sockaddr *)&clientAdresse, (socklen_t *)&addrLen)) != -1)
    {
        // Convertion de l'IP en texte
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAdresse.sin_addr), ip, INET_ADDRSTRLEN);
        printf("[INFO - SERVER] Connexion de %s:%i\n", ip, clientAdresse.sin_port);

        // Paramètrage de la socket (mode non bloquant)
        int opt = 1;
        setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(1));
    }
    return clientSocket;
}