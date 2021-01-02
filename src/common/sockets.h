#ifndef DEF_SOCKET_H
#define DEF_SOCKET_H

// Port d'écoute de la socket
#define PORT 6000
// Taille du tampon de lecture des messages
#define BUFFER_LEN 200
// Commande pour arrêter le socket
#define EXIT_WORD "exit"

int initSocket(struct sockaddr_in *);

#endif //DEF_SOCKET_H