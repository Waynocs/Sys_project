/**
 * @ Author: SUBLET Tom
 * @ Create Time: 2021-01-07 17:30:55
 * @ Description: common names
 */

#ifndef DEF_WORDS_H
#define DEF_WORDS_H

// Réponse à attendre du serveur
#define READY_WORD "ready"
// TODO comment
#define SEE_PLACES_IN_WORD "seeplaces"
// TODO comment
#define SEE_TAKEN_PLACES_IN_WORD "seetakenplaces"
// TODO comment
#define NEW_PLACE_IN_WORD "newplace"
// TODO comment
#define CANCEL_IN_WORD "cancel"
// Réponse à attendre du serveur
#define READY_OUT_WORD "ready"
// TODO comment
#define SUCCESS_OUT_WORD "ok"
// TODO comment
#define TAKEN_ERROR_OUT_WORD "taken"
// TODO comment
#define FNAME_ERROR_OUT_WORD "fname"
// TODO comment
#define NAME_ERROR_OUT_WORD "name"
// TODO comment
#define NODOSS_ERROR_OUT_WORD "nodoss"
// TODO comment
#define EXIST_ERROR_OUT_WORD "exist"
// TODO comment
#define NO_CMD_ERROR_OUT_WORD "wat"
// Commande pour arrêter le socket
#define EXIT_IN_WORD "exit"

int initSocket(struct sockaddr_in *);

#endif //DEF_WORDS_H