/**
 * @ Author: SUBLET Tom & SERANO Waïan
 * @ Create Time: 2021-01-02 00:30:36
 * @ Description: The client
 */

#ifndef DEF_CLIENT_H
#define DEF_CLIENT_H

// Nombre de places dans la salle
#define NB_MAX_SALLE 100

void initAdresse(struct sockaddr_in *);
int initClientSocket(struct sockaddr_in *);
int manageServer(int);

#endif //DEF_CLIENT_H