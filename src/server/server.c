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
#include "server.h"

struct Salle salle;
struct Client clients[NB_CLIENTS];
int isRunning = 1;

void exitHandler(int dummy)
{
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
            printf("Attribution de l'identifiant\n");
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
                        printf("[Client %d] Error thread\n", clients[i].id);
                    }

                    isEmptyPlace = 1;
                    break;
                }
            }
            if (!isEmptyPlace)
            {
                printf("Plus de place");
            }
        }
    }
    return EXIT_SUCCESS;
}

int clientMain(struct Client *client)
{
    printf("[Client %d] Création du thread\n", client->id);
    send(client->socket, "Entrez 'exit' pour quitter\n", strlen("Entrez 'exit' pour quitter\n"), MSG_DONTWAIT);

    int isClosed = 0;

    while (isRunning && !isClosed)
    {
        isClosed = manageClient(client);
    }

    printf("[Client %d] Fin du thread\n", client->id);
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
        printf("[Client %d] Errno {%i} : %s\n", client->id, errno, strerror(errno));
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
        printf("[INFO - Client %d] < %s\n", client->id, buffer);
        manageCommands(client, buffer);
    }

    if (isClosed == 1)
    {
        // La socket est fermé ou le client veut quitter le serveur !
        printf("[Client %d] Fermeture de la connexion\n", client->id);
        // Fermeture de la socket
        close(client->socket);
    }

    return isClosed;
}

void manageCommands(struct Client *client, char *buffer)
{
    char *response = malloc(sizeof(char));
    char *command = strtok(buffer, "_");

    if (strncmp(buffer, EXIT_WORD, 4) == 0)
    {
        // Le client veut se déconnecter
        send(client->socket, "Bye\0", strlen("Bye\0"), 0);
        // La socket est fermé ou le client veut quitter le serveur !
        printf("[Client %d] Fermeture de la connexion\n", client->id);
        // Fermeture de la socket
        close(client->socket);
        return;
    }
    else if (strncmp(command, "seeplaces", strlen("seeplaces")) == 0)
    {
        sprintf(response, "%d", NB_MAX_SALLE - salle.nbNonLibres);
        printf("[INFO - Client %d] > %s\n", client->id, response);
    }
    else if (strncmp(command, "seetakenplaces", strlen("seetakenplaces")) == 0)
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
        response = malloc(len * sizeof(char));
        strcpy(response, places);
        printf("[INFO - Client %d] takenplaces: %s | %s\n", client->id, places, response);
        free(places);
    }
    else if (strncmp(command, "newplace", strlen("newplace")) == 0)
    {
        struct Place place;
        int index = salle.nbNonLibres;
        int isPlaceError = 0;

        char *name = strtok(NULL, "_");        // Get the first parameter
        char *fname = strtok(NULL, "_");       // Get the second parameter
        char *chosenPlace = strtok(NULL, "_"); // Get the third parameter
        if (chosenPlace != NULL)
        {
            index = atoi(chosenPlace);
            if (salle.places[index].noDoss != NULL)
            {
                strcpy(response, "place taken");
                printf("[ERRO - Client %d] place is aldreay taken\n", client->id);
                isPlaceError = 1;
            }
        }

        if (!isPlaceError)
        {
            printf("1\n");
            if (name != NULL)
            {
                int len = strlen(name);
                place.nom = malloc(len * sizeof(char));
                strcpy(place.nom, name);
                printf("2\n");

                if (fname != NULL)
                {
                    len = strlen(fname);
                    place.prenom = malloc(len * sizeof(char));
                    strcpy(place.prenom, fname);
                    printf("3\n");

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
                    printf("4\n");

                    len = strlen(noDoss);
                    place.noDoss = malloc(len * sizeof(char));
                    strcpy(place.noDoss, noDoss);
                    printf("5\n");

                    printf("[INFO - Client %d] > %s - %s - %s - %d \n", client->id, name, fname, noDoss, index);

                    salle.places[index] = place;
                    salle.nbNonLibres++;
                    printf("6\n");

                    strcpy(response, noDoss);
                    printf("7\n");
                }
                else
                {
                    printf("21\n");
                    strcpy(response, "fname null");
                    printf("[ERRO - Client %d] fName is null\n", client->id);
                }
            }
            else
            {
                printf("11\n");
                strcpy(response, "name null");
                printf("[ERRO - Client %d] Name is null\n", client->id);
            }
        }
    }
    else if (strncmp(command, "cancel", strlen("cancel")) == 0)
    {
        char *name = strtok(NULL, "_");
        char *noDoss = strtok(NULL, "_");

        int index = -1;

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
            int len = strlen("ok");
            response = malloc(len * sizeof(char) + 2);
            printf("[INFO - Client %d] > %s - %s supprimé \n", client->id, name, noDoss);
            strcpy(response, "ok");
            salle.nbNonLibres--;
        }
        else
        {
            strcpy(response, "dont exist");
            printf("[ERRO - Client %d] Client doesn't exist\n", client->id);
        }
    }
    else
    {
        // On renvoie le texte au client dans un buffer assez grand
        int len = strlen("Vous avez dit : ") + strlen(buffer) + 1;
        response = malloc(len * sizeof(char) + 2);
        strcpy(response, "Vous avez dit : ");
        strcat(response, buffer);
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
    printf("Attachement de la socket sur le port %i\n", PORT);
    if (bind(fdsocket, (struct sockaddr *)adresse, sizeof(*adresse)) != 0)
    {
        printf("Echéc d'attachement: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Passage en écoute de la socket
    printf("Mise en écoute de la socket\n");
    if (listen(fdsocket, BACKLOG) != 0)
    {
        printf("Echec de la mise en écoute: %s\n", strerror(errno));
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
        printf("Connexion de %s:%i\n", ip, clientAdresse.sin_port);

        // Paramètrage de la socket (mode non bloquant)
        int opt = 1;
        setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(1));
    }
    return clientSocket;
}