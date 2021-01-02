#ifndef DEF_SERVER_H
#define DEF_SERVER_H

// Adresse d'écoute (toutes les adresses)
#define IP INADDR_ANY
// Taille de la file d'attente
#define BACKLOG 3
// Nombre de connexions clients
#define NB_CLIENTS 10

void initAdresse(struct sockaddr_in *);
int initServerSocket(struct sockaddr_in *);
int waitForClient(int *);
void addClientToTab(int, int[]);
void manageClient(int[]);
void manageCommands(int, char[]);

#endif //DEF_SERVER_H