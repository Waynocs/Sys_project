#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "sockets.h"

// Paramétrage de la socket
int initSocket(struct sockaddr_in *adresse)
{
    // Descripteur de socket
    int fdsocket;
    // Nombre d'options
    int opt = 1;

    // Création de la socket en TCP
    printf("Création de la socket\n");
    if ((fdsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Echéc de la création: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Paramètrage de la socket
    printf("Paramètrage de la socket\n");
    if (setsockopt(fdsocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0)
    {
        printf("Echéc de paramètrage: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fdsocket;
}