#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 6000
#define MAX_BUFFER 1000
#define MAX_CLIENTS 100
#define NB 10

typedef struct
{
    int tableau[NB];
} Variables;

void initialiserTableau(int tab[]);
void *afficherTableau(void *arg);

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
    pthread_t monThread;
    int fdWaitSocket;
    int fdCommunicationSocket;
    struct sockaddr_in serverCoor;
    struct sockaddr_in appellantCoor;
    char tampon[MAX_BUFFER];
    int NbRnbReceived;
    int addressLenght;
    Variables donnees;

    printf("Initialisation du tableau\n");
    initialiserTableau(donnees.tableau);

    if (pthread_create(&monThread, NULL, afficherTableau, &donnees) != 0)
    {
        printf("erreur de creation de thread\n");
        return EXIT_FAILURE;
    }

    printf("Creation du thread -> ok\n");
    pthread_join(monThread, NULL);

    return EXIT_SUCCESS;

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

void initialiserTableau(int tab[])
{
    for (int i = 0; i < NB; i++)
    {
        tab[i] = i * i;
    }
}

void *afficherTableau(void *arg)
{
    Variables *structure = (Variables *)arg;
    printf("Je suis un thread et voici les valeurs de tableau :\n");

    for (int i = 0; i < NB; i++)
    {
        printf("%d\n", structure->tableau[i]);
    }

    pthread_exit(NULL);
}