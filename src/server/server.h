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
// Nombre de places dans la salle
#define NB_MAX_SALLE 100

// Un client connecté au serveur
struct Client
{
    // Le socket lié au client
    int socket;
    // L'id du client, utilisé pour controler le nombre de client connecté
    int id;
};

// Une reservation
struct Place
{
    // Nom de la reservation
    char *nom;
    // Prénom de la reservation
    char *prenom;
    // Numéro de la reservation
    char *noDoss;
};

// La salle de concert
struct Salle
{
    // Les places de la salle
    struct Place *places;
    // Le nombre de places non disponibles
    int nbNonLibres;
};

int initServerSocket(struct sockaddr_in *);
int waitForClient(int *);
int clientMain(struct Client *);
int manageClient(struct Client *);
void manageCommands(struct Client *, char[]);

#endif //DEF_SERVER_H