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
#include "client.h"

void readMessage(char *tampon[])
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
    msleep(10);
    while ((isClosed = manageServer(clientSocket)) == 0)
    {
        readMessage(buffer);
        send(clientSocket, buffer, strlen(buffer), MSG_DONTWAIT);
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
    int fdsocket = initSocket(&adresse);

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
        system("clear");
        printf("%s\n", buffer);
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