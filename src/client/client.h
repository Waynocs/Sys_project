#ifndef DEF_CLIENT_H
#define DEF_CLIENT_H

void initAdresse(struct sockaddr_in *);
int initClientSocket(struct sockaddr_in *);
int manageServer(int);

#endif //DEF_CLIENT_H