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
int isRunning = 1;

void exitHandler(int dummy)
{
    isRunning = 0;
    for (int i = 0; i < NB_MAX_SALLE; i++)
    {
        free(salle.places[i].nom);
        free(salle.places[i].prenom);
        free(salle.places[i].noDoss);
    }

    free(salle.places);
}

int main(void)
{
    int noCli = 0;
    struct Client clients[NB_CLIENTS];

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
    while (1)
    {
        // Descripteur de la socket du client, on attend une connexion
        if ((clientSocket = waitForClient(&serverSocket)) != -1)
        {
            if (noCli < NB_CLIENTS)
            {
                struct Client client;
                client.socket = clientSocket;
                client.id = noCli;
                clients[noCli] = client;

                // On attribue un thread au nouveau client
                thrd_t thread;
                if (thrd_create(&thread, clientMain, &(clients[noCli])) != thrd_success)
                {
                    printf("[Client %d] Error thread\n", clients[noCli].id);
                }
                noCli++;
            }
            else
            {
                printf("Plus de place !\n");
            }
        }
    }
    return EXIT_SUCCESS;
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
        //TODO: TODO
    }
    else if (strncmp(command, "newplace", strlen("newplace")) == 0)
    {
        struct Place place;

        char *name = strtok(NULL, "_");
        char *fname = strtok(NULL, "_");
        if (name != NULL)
        {
            int len = strlen(name);
            place.nom = malloc(len * sizeof(char));
            memcpy(place.nom, name, sizeof(name));

            if (fname != NULL)
            {
                len = strlen(fname);
                place.prenom = malloc(len * sizeof(char));
                memcpy(place.prenom, fname, sizeof(fname));

                strcpy(response, "1111111111"); //TODO: gen

                //TODO: check if noDoss exists
                len = strlen(response);
                place.noDoss = malloc(len * sizeof(char));
                memcpy(place.noDoss, response, sizeof(fname));

                printf("[INFO - Client %d] > %s - %s - %s \n", client->id, name, fname, response);

                salle.places[salle.nbNonLibres] = place;
                salle.nbNonLibres++;
            }
            else
            {
                strcpy(response, "fname null");
                printf("[ERRO - Client %d] fName is null\n", client->id);
            }
        }
        else
        {
            strcpy(response, "name null");
            printf("[ERRO - Client %d] Name is null\n", client->id);
        }
    }
    else if (strncmp(command, "cancel", strlen("cancel")) == 0)
    {
        char *name = strtok(NULL, "_");
        char *noDoss = strtok(NULL, "_");

        int len = strlen("ok");
        response = malloc(len * sizeof(char) + 2);
        printf("[INFO - Client %d] > %s - %s supprimé \n", client->id, name, noDoss);
        strcpy(response, "ok");

        //TODO: TODO

        salle.nbNonLibres--;
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