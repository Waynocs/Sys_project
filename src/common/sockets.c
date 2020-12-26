#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include "sockets.h"

struct sockaddr_in createCoordsServ(int port)
{
    struct sockaddr_in coordonneesServeur;

    // On prépare les coordonnées du serveur
    memset(&coordonneesServeur, 0, sizeof(struct sockaddr_in));

    coordonneesServeur.sin_family = PF_INET;
    // toutes les interfaces locales disponibles
    coordonneesServeur.sin_port = htons(port);

    return coordonneesServeur;
}