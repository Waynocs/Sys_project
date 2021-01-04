/**
 * @ Author: SUBLET Tom & SERANO Waïan
 * @ Create Time: 2021-01-02 00:30:36
 * @ Description: The server
 */

#ifndef DEF_SERVER_H
#define DEF_SERVER_H

// Adresse d'écoute (toutes les adresses)
#define IP INADDR_ANY
// Taille de la file d'attente
#define BACKLOG 3
// Nombre de connexions clients
#define NB_CLIENTS 10

#define NB_MAX_SALLE 100

struct Client
{
    int socket;
    int id;
};

struct Place
{
    char *nom;
    char *prenom;
    char *noDoss;
};

struct Salle
{
    struct Place *places;
    int nbNonLibres;
};

int initServerSocket(struct sockaddr_in *);
int waitForClient(int *);
int clientMain(struct Client *);
int manageClient(struct Client *);
void manageCommands(struct Client *, char[]);

#endif //DEF_SERVER_H