#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 6000
#define MAX_BUFFER 1000

const char *EXIT = "exit";

void readMessage(char tampon[])
{
    printf("Saisir un message à envoyer :\n");
    fgets(tampon, MAX_BUFFER, stdin);
    strtok(tampon, "\n");
}

int testExit(char tampon[])
{
    return strcmp(tampon, EXIT) == 0;
}

int main(int argc, char const *argv[])
{
    int fdSocket;
    int nbReceived;
    struct sockaddr_in serverCoor;
    int addressLength;
    char tampon[MAX_BUFFER];

    fdSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (fdSocket < 0)
    {
        printf("socket incorrecte\n");
        exit(EXIT_FAILURE);
    }

    // On prépare les coordonnées du serveur
    addressLength = sizeof(struct sockaddr_in);
    memset(&serverCoor, 0x00, addressLength);

    serverCoor.sin_family = PF_INET;
    // adresse du serveur
    inet_aton("127.0.0.1", &serverCoor.sin_addr);
    // toutes les interfaces locales disponibles
    serverCoor.sin_port = htons(PORT);

    if (connect(fdSocket, (struct sockaddr *)&serverCoor, sizeof(serverCoor)) == -1)
    {
        printf("connexion impossible\n");
        exit(EXIT_FAILURE);
    }

    printf("connexion ok\n");

    while (1)
    {
        readMessage(tampon);

        if (testExit(tampon))
        {
            send(fdSocket, tampon, strlen(tampon), 0);
            break; // on quitte la boucle
        }

        // on envoie le message au serveur
        send(fdSocket, tampon, strlen(tampon), 0);

        // on attend la réponse du serveur
        nbReceived = recv(fdSocket, tampon, MAX_BUFFER, 0);

        if (nbReceived > 0)
        {
            tampon[nbReceived] = 0;
            printf("Recu : %s\n", tampon);

            if (testExit(tampon))
            {
                break; // on quitte la boucle
            }
        }
    }

    close(fdSocket);

    return EXIT_SUCCESS;
}