#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"

#define PORT 6000
#define MAX_BUFFER 1000

int ouvrirUneConnexionTcp()
{
    struct sockaddr_in coordonneesServeur = createCoordsServ(PORT);

    int socketTemp = socket(AF_INET, SOCK_STREAM, 0);

    if (socket < 0)
    {
        printf("socket incorrect\n");
        exit(EXIT_FAILURE);
    }

    // adresse du serveur
    inet_aton("127.0.0.1", &coordonneesServeur.sin_addr);

    if (connect(socketTemp, (struct sockaddr *)&coordonneesServeur, sizeof(coordonneesServeur)) == -1)
    {
        printf("connexion impossible\n");
        exit(EXIT_FAILURE);
    }

    printf("connexion ok\n");

    return socketTemp;
}

int main(int argc, char const *argv[])
{
    int fdSocket;
    int nbRecu;
    char tampon[MAX_BUFFER];

    fdSocket = ouvrirUneConnexionTcp();

    printf("Envoi du message au serveur.\n");
    strcpy(tampon, "Message du client vers le serveur");
    send(fdSocket, tampon, strlen(tampon), 0);

    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0); // on attend la réponse du serveur

    if (nbRecu > 0)
    {
        tampon[nbRecu] = 0;
        printf("Recu : %s\n", tampon);
    }

    close(fdSocket);

    return EXIT_SUCCESS;
}