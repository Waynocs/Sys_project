/**
 * @ Author: SUBLET Tom & SERANO Waïan
 * @ Create Time: 2021-01-02 00:30:36
 * @ Description: The common sockets
 */

#ifndef DEF_SOCKET_H
#define DEF_SOCKET_H

// Port d'écoute de la socket
#define PORT 6000
// Taille du tampon de lecture des messages
#define BUFFER_LEN 200

/*
 * Crée un socket
 * 
 * @param a L'adresse du socket
 * 
 * @return Le socket
*/
int initSocket(struct sockaddr_in *a);

#endif //DEF_SOCKET_H