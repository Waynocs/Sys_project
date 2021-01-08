/**
 * @ Author: SUBLET Tom & SERANO Waïan
 * @ Create Time: 2021-01-07 17:30:55
 * @ Description: common names
 */

#ifndef DEF_WORDS_H
#define DEF_WORDS_H

/*
 * Commande pour afficher le nombre de places disponibles
*/
#define SEE_PLACES_IN_WORD "seeplaces"

/*
 * Commande pour afficher les places déjà reservées.
 * @return string de type '-_ID_ID_...'
*/
#define SEE_TAKEN_PLACES_IN_WORD "seetakenplaces"

/*
 * Commande pour demander une place.
 * 
 * @param name Le nom de famille du client.
 * @param fname Le prénom du client.
 * @param id [Optionnel] La place choisie.
 * 
 * @return Le numéro de dossier
 * 
 * @exception NAME_ERROR_OUT_WORD: Si le nom donné est null
 * @exception FNAME_ERROR_OUT_WORD: Si le prénom donné est null
 * @exception TAKEN_ERROR_OUT_WORD: Si la place est déjà prise
*/
#define NEW_PLACE_IN_WORD "newplace"

/*
 * Commande pour annuler une place.
 * 
 * @param fname Le prénom du client.
 * @param doss Le numéro de dossier du client.
 * 
 * @return SUCCESS_OUT_WORD
 * 
 * @excetpion NAME_ERROR_OUT_WORD: Si le nom donné est null
 * @exception NODOSS_ERROR_OUT_WORD: Si le numéro de dossier est null
 * @exception EXIST_ERROR_OUT_WORD: Si client n'existe pas
*/
#define CANCEL_IN_WORD "cancel"

/*
 * Réponse à attendre du serveur avant de communiquer.
*/
#define READY_OUT_WORD "ready"

/*
 * Réponse du serveur en cas de succès
*/
#define SUCCESS_OUT_WORD "ok"

/*
 * Réponse du serveur en cas d'erreur.
 * La place demandée est déjà reservée.
*/
#define TAKEN_ERROR_OUT_WORD "taken"

/*
 * Réponse du serveur en cas d'erreur.
 * Le prénom donné est null.
*/
#define FNAME_ERROR_OUT_WORD "fname"

/*
 * Réponse du serveur en cas d'erreur.
 * Le nom donné est null.
*/
#define NAME_ERROR_OUT_WORD "name"

/*
 * Réponse du serveur en cas d'erreur.
 * Le numéro de dossier donné est null.
*/
#define NODOSS_ERROR_OUT_WORD "nodoss"

/*
 * Réponse du serveur en cas d'erreur.
 * Le client n'existe pas.
*/
#define EXIST_ERROR_OUT_WORD "exist"

/*
 * Réponse du serveur en cas d'erreur.
 * La comande n'a pas été comprise.
*/
#define NO_CMD_ERROR_OUT_WORD "wat"

/*
 * Commande pour arrêter le socket
*/
#define EXIT_IN_WORD "exit"
/*
 * Réponse du serveur pour arrêter le socket
*/
#define EXIT_OUT_WORD "bye"

int initSocket(struct sockaddr_in *);

#endif //DEF_WORDS_H