#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 6000
#define MAX_BUFFER 1000
#define MAX_CLIENTS 100

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
    int fdWaitSocket;
    int fdCommunicationSocket;
    struct sockaddr_in serverCoor;
    struct sockaddr_in appellantCoor;
    char tampon[MAX_BUFFER];
    int NbRnbReceived;
    int addressLenght;
    int pid;

    fdWaitSocket = socket(PF_INET, SOCK_STREAM, 0);

    if (fdWaitSocket < 0)
    {
        printf("socket incorrecte\n");
        exit(EXIT_FAILURE);
    }

    // On prépare l’adresse d’attachement locale
    addressLenght = sizeof(struct sockaddr_in);
    memset(&serverCoor, 0x00, addressLenght);

    serverCoor.sin_family = PF_INET;
    // toutes les interfaces locales disponibles
    serverCoor.sin_addr.s_addr = htonl(INADDR_ANY);
    // toutes les interfaces locales disponibles
    serverCoor.sin_port = htons(PORT);

    if (bind(fdWaitSocket, (struct sockaddr *)&serverCoor, sizeof(serverCoor)) == -1)
    {
        printf("erreur de bind\n");
        exit(EXIT_FAILURE);
    }

    if (listen(fdWaitSocket, 5) == -1)
    {
        printf("erreur de listen\n");
        exit(EXIT_FAILURE);
    }

    socklen_t coorSize = sizeof(appellantCoor);

    int nbCustomers = 0;

    while (nbCustomers < MAX_CLIENTS)
    {
        if ((fdCommunicationSocket = accept(fdWaitSocket, (struct sockaddr *)&appellantCoor,
                                            &coorSize)) == -1)
        {
            printf("erreur de accept\n");
            exit(EXIT_FAILURE);
        }

        printf("Client connecté - %s:%d\n",
               inet_ntoa(appellantCoor.sin_addr),
               ntohs(appellantCoor.sin_port));

        if ((pid = fork()) == 0)
        {
            close(fdWaitSocket);

            while (1)
            {
                // on attend le message du client
                // la fonction recv est bloquante
                NbRnbReceived = recv(fdCommunicationSocket, tampon, MAX_BUFFER, 0);

                if (NbRnbReceived > 0)
                {
                    tampon[NbRnbReceived] = 0;
                    printf("Recu de %s:%d : %s\n",
                           inet_ntoa(appellantCoor.sin_addr),
                           ntohs(appellantCoor.sin_port),
                           tampon);

                    if (testExit(tampon))
                    {
                        break; // on quitte la boucle
                    }
                }

                readMessage(tampon);

                if (testExit(tampon))
                {
                    send(fdCommunicationSocket, tampon, strlen(tampon), 0);
                    break; // on quitte la boucle
                }

                // on envoie le message au client
                send(fdCommunicationSocket, tampon, strlen(tampon), 0);
            }

            exit(EXIT_SUCCESS);
        }

        nbCustomers++;
    }

    close(fdCommunicationSocket);
    close(fdWaitSocket);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        wait(NULL);
    }

    printf("Fin du programme.\n");
    return EXIT_SUCCESS;
}