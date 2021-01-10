/**
 * @ Author: SUBLET Tom & SERANO Waïan
 * @ Create Time: 2021-01-02 00:30:36
 * @ Description: The server
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <threads.h>

#include "../common/sockets.h"
#include "../common/words.h"
#include "server.h"

// La salle de concert
struct Salle salle;
// Les clients connectés, utilisé pour les threads
struct Client clients[NB_CLIENTS];
// Variable coupant tout les threads si mit à 0
int isRunning = 1;

// Evite une fuite de mémoire en cas de coupure du serveur
void exitHandler(int dummy)
{
    //TODO: double free or corruption (!prev)
    // On coupe tout les threads
    isRunning = 0;
    // Pour toute les places de la salle...
    for (int i = 0; i < NB_MAX_SALLE; i++)
    {
        // Si elle possède un numéro de dossier (donc qu'elle a été reservée)
        if (salle.places[i].noDoss != NULL)
        {
            // On libére la mémoire
            free(salle.places[i].nom);
            free(salle.places[i].prenom);
            free(salle.places[i].noDoss);
        }
    }

    // On libère toute les places
    free(salle.places);
}

int main(void)
{
    // Initialise l'aléatoire
    srand((unsigned int)time(NULL));

    // On initialise le tableau de client
    for (int i = 0; i < NB_MAX_SALLE; i++)
    {
        clients[i].id = -1;
        clients[i].socket = -1;
    }

    // On initialise les places de la salle
    salle.places = malloc(NB_MAX_SALLE * sizeof(struct Place));
    salle.nbNonLibres = 0;

    // Si le serveur est arrété, on évite les fuites de mémoire
    signal(SIGINT, exitHandler);

    struct sockaddr_in adresse;
    // Initialisation de l'adresse serveur
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = IP;
    adresse.sin_port = htons(PORT);

    // On initialise le socket du serveur
    int serverSocket = initServerSocket(&adresse);

    int clientSocket;
    int isEmptyPlace = 0;
    while (1)
    {
        // Descripteur de la socket du client, on attend une connexion
        if ((clientSocket = waitForClient(&serverSocket)) != -1)
        {
            printf("[INFO - SERVER] Attribution de l'identifiant\n");
            for (int i = 0; i < NB_CLIENTS; i++)
            {
                // Si la place i est disponible
                if (clients[i].id == -1)
                {
                    // On créé un nouveau client
                    struct Client client;
                    client.socket = clientSocket;
                    client.id = i;
                    clients[i] = client;

                    // On attribue un thread au nouveau client
                    thrd_t thread;
                    if (thrd_create(&thread, clientMain, &(clients[i])) != thrd_success)
                    {
                        printf("[ERROR - CLIENT %d] Error thread\n", clients[i].id);
                    }

                    // Il y avait une place libre
                    isEmptyPlace = 1;
                    break;
                }
            }
            if (!isEmptyPlace)
            {
                printf("[INFO - SERVER] Plus de place");
            }
        }
    }

    return EXIT_SUCCESS;
}

int clientMain(struct Client *client)
{
    printf("[INFO - CLIENT %d] Création du thread\n", client->id);
    // On envoie au client qu'on est pret à recevoir des commandes
    send(client->socket, READY_OUT_WORD, strlen(READY_OUT_WORD), MSG_DONTWAIT);

    int isClosed = 0;
    // Tant que les threads sont autorisé et que la connexion n'est pas fermée
    while (isRunning && !isClosed)
    {
        // On traite le message du client
        isClosed = manageClient(client);
    }

    printf("[INFO - CLIENT %d] Fin du thread\n", client->id);
    // On libère la place
    clients[client->id].id = -1;

    return thrd_success;
}

// On traite l'input des clients
int manageClient(struct Client *client)
{
    // Création d'un tampon pour stocker les messages des clients dans la heap
    static char buffer[BUFFER_LEN + 1];

    // On récupère le message venant du client
    int len = recv(client->socket, buffer, BUFFER_LEN, MSG_DONTWAIT);

    // Booléen pour suivre l'état de la socket
    int isClosed = 0;
    // Si la réponse n'existe pas
    if (len == -1 && errno != EAGAIN)
    {
        // Une erreur est survenue
        printf("[ERROR - CLIENT %d] Errno {%i} : %s\n", client->id, errno, strerror(errno));
        isClosed = 1;
    }
    // Si ma réponse est nulle
    else if (len == 0)
    {
        // Le client s'est déconnecté (extrémité de la socket fermée)
        isClosed = 1;
    }
    // Si la réponse existe
    else if (len > 0)
    {
        // Ajout du terminateur de chaîne
        buffer[len] = '\0';
        printf("[INFO - CLIENT %d] Commande : %s\n", client->id, buffer);
        // On gére le comportement en fonction de la commande
        manageCommands(client, buffer);
    }

    if (isClosed == 1)
    {
        // La socket est fermé ou le client veut quitter le serveur !
        printf("[INFO - CLIENT %d] Fermeture de la connexion\n", client->id);
        // Fermeture de la socket
        close(client->socket);
    }

    return isClosed;
}

void manageCommands(struct Client *client, char *buffer)
{
    // On prépare la réponse du serveur
    char *response = malloc(sizeof(char));
    // On récupère la commande sans paramètres
    char *command = strtok(buffer, "_");

    // Si la commande correspont à EXIT_IN_WORD
    if (strncmp(buffer, EXIT_IN_WORD, strlen(EXIT_IN_WORD)) == 0)
    {
        // Le client veut se déconnecter
        send(client->socket, EXIT_OUT_WORD, strlen(EXIT_OUT_WORD), 0);
        // La socket est fermé ou le client veut quitter le serveur !
        printf("[INFO - CLIENT %d] Fermeture de la connexion\n", client->id);
        // Fermeture de la socket
        close(client->socket);

        // On passe la fin de la fonction
        return;
    }
    // Si la commande correspont à SEE_PLACES_IN_WORD
    else if (strncmp(command, SEE_PLACES_IN_WORD, strlen(SEE_PLACES_IN_WORD)) == 0)
    {
        // On met dans la réponse le nombre de places disponibles
        sprintf(response, "%d", NB_MAX_SALLE - salle.nbNonLibres);
        printf("[INFO - CLIENT %d] Consultation des places dispos (Réponse: %s)\n", client->id, response);
    }
    // Si la commande correspont à SEE_TAKEN_PLACES_IN_WORD
    else if (strncmp(command, SEE_TAKEN_PLACES_IN_WORD, strlen(SEE_TAKEN_PLACES_IN_WORD)) == 0)
    {
        // On prépare la liste des places
        char *places = malloc(sizeof(char));
        // On fait attention à ce que la réponse ne soit pas vide
        strcpy(places, "-");
        // Pour chaque place
        for (int i = 0; i < NB_MAX_SALLE; i++)
        {
            // Si elle est reservée
            if (salle.places[i].noDoss != NULL)
            {
                // On rajoutte à la liste l'id de la place
                sprintf(places, "%s_%d", places, i);
            }
        }

        // On copie la liste dans la réponse
        int len = strlen(places);
        response = malloc(len * sizeof(char) + 2);
        strcpy(response, places);

        printf("[INFO - CLIENT %d] Consultation des places prises (Réponse: %s)\n", client->id, response);

        // On évite une fuite de mémoire
        free(places);
    }
    // Si la commande correspont à NEW_PLACE_IN_WORD
    else if (strncmp(command, NEW_PLACE_IN_WORD, strlen(NEW_PLACE_IN_WORD)) == 0)
    {
        struct Place place;
        int index = -1;
        int isPlaceError = 0;

        // On récupère le premier paramètre (le nom)
        char *name = strtok(NULL, "_");
        // On récupère le deuxième paramètre (le prénom)
        char *fname = strtok(NULL, "_");
        // On récupère le troisière paramètre (la place choisie)
        char *chosenPlace = strtok(NULL, "_");

        // Si la place choisie n'est pas nulle
        if (chosenPlace != NULL)
        {
            int isStringError = 0;
            // On vérifie si le paramètre est bien un entier
            for (int i = 0; i < strlen(chosenPlace); i++)
            {
                if (chosenPlace[i] < 48 || chosenPlace[i] > 57)
                {
                    isStringError = 1;
                    break;
                }
            }

            if (!isStringError)
            {
                // On transforme le paramètre en entier
                index = atoi(chosenPlace);
            }

            // Si la place est reservée
            if (salle.places[index].noDoss != NULL || isStringError)
            {
                strcpy(response, TAKEN_ERROR_OUT_WORD);
                printf("[ERROR - CLIENT %d] Demande une place... La place %d est déjà prise\n", client->id, index);
                isPlaceError = 1;
            }
            else
            {
                // Sinon on bloque la place
                int len = strlen("reserved");
                salle.places[index].noDoss = malloc(len * sizeof(char));
                strcpy(salle.places[index].noDoss, "reserved");
            }
        }

        // Si la place n'est pas reservée
        if (!isPlaceError)
        {
            //Si le nom existe
            if (name != NULL)
            {
                // On met le nom dans la place
                int len = strlen(name);
                place.nom = malloc(len * sizeof(char));
                strcpy(place.nom, name);

                // Si le nom existe
                if (fname != NULL)
                {
                    // On met le prénom dans la place
                    len = strlen(fname);
                    place.prenom = malloc(len * sizeof(char));
                    strcpy(place.prenom, fname);

                    int isAlreadySet = 0;
                    char noDoss[10];

                    do
                    {
                        // On génére un string à 10 chiffres
                        for (int i = 0; i < 10; i++)
                        {
                            noDoss[i] = (rand() % 10) + 48;
                        }
                        noDoss[10] = '\0';

                        // On vérifie que le numéro de dossier n'existe pas
                        for (int i = 0; i < NB_MAX_SALLE; i++)
                        {
                            if (salle.places[i].noDoss != NULL && strcmp(salle.places[i].noDoss, noDoss) == 0)
                            {
                                isAlreadySet = 1;
                                break;
                            }
                        }
                    } while (isAlreadySet);

                    // On met le numéro de dossier dans la place
                    len = strlen(noDoss);
                    place.noDoss = malloc(len * sizeof(char));
                    strcpy(place.noDoss, noDoss);

                    // Si la place n'a pas été choisie
                    if (index == -1)
                    {
                        // On prend la dernière place disponible
                        index = salle.nbNonLibres;
                    }
                    // On enregistre la place
                    salle.places[index] = place;
                    salle.nbNonLibres++;
                    printf("[INFO - CLIENT %d] %s %s crée le dossier %s à la place %d\n", client->id, name, fname, noDoss, index);

                    // On copie le numéro de dossier dans la réponse
                    strcpy(response, noDoss);
                }
                // Si le prénom est null
                else
                {
                    strcpy(response, FNAME_ERROR_OUT_WORD);
                    printf("[ERROR - CLIENT %d] Le fName donné est null\n", client->id);
                }
            }
            // Si le nom est null
            else
            {
                strcpy(response, NAME_ERROR_OUT_WORD);
                printf("[ERROR - CLIENT %d] Le name donné est null\n", client->id);
            }
        }
    }
    // Si la commande correspont à CANCEL_IN_WORD
    else if (strncmp(command, CANCEL_IN_WORD, strlen(CANCEL_IN_WORD)) == 0)
    {
        // On récupère le premier paramètre (le nom)
        char *name = strtok(NULL, "_");
        // On récupère le premier paramètre (le numéro de dossier)
        char *noDoss = strtok(NULL, "_");

        int index = -1;

        // Si le nom n'est pas null
        if (name != NULL)
        {
            // Si le numéro de dossier n'est pas null
            if (noDoss != NULL)
            {
                for (int i = 0; i < NB_MAX_SALLE; i++)
                {
                    // Si place n'est pas vide
                    if (salle.places[i].noDoss != NULL)
                    {
                        // On vérifie que le numéro de dossier et nom correspond
                        if (strcmp(salle.places[i].noDoss, noDoss) == 0 && strcmp(salle.places[i].nom, name) == 0)
                        {
                            index = i;
                            break;
                        }
                    }
                }

                // Si une place a été trouvée
                if (index > -1)
                {
                    // On libère la place
                    salle.places[index].nom = NULL;
                    salle.places[index].prenom = NULL;
                    salle.places[index].noDoss = NULL;
                    // On envoie le message comme quoi tout c'est bien passé
                    int len = strlen(SUCCESS_OUT_WORD);
                    response = malloc(len * sizeof(char) + 2);
                    printf("[INFO - CLIENT %d] %s - %s supprimé\n", client->id, name, noDoss);
                    strcpy(response, SUCCESS_OUT_WORD);
                    // On augment le nombre de place disponible
                    salle.nbNonLibres--;
                }
                else
                {
                    strcpy(response, EXIST_ERROR_OUT_WORD);
                    printf("[ERROR - CLIENT %d] Le dossier n'existe pas\n", client->id);
                }
            }
            // Si le numéro de dossier est null
            else
            {
                strcpy(response, NODOSS_ERROR_OUT_WORD);
                printf("[ERROR - CLIENT %d] Le noDoss donné est null\n", client->id);
            }
        }
        // Si le nom est null
        else
        {
            strcpy(response, NAME_ERROR_OUT_WORD);
            printf("[ERROR - CLIENT %d] Le name donné est null\n", client->id);
        }
    }
    // Si la commande est autre
    else
    {
        // On revoie une erreeur
        int len = strlen(NO_CMD_ERROR_OUT_WORD);
        response = malloc(len * sizeof(char) + 2);
        strcpy(response, NO_CMD_ERROR_OUT_WORD);
        printf("[ERROR - CLIENT %d] Commande non comprise\n", client->id);
    }
    // Un seul envoi permet de ne pas surcharger le réseau
    strcat(response, "\0");
    send(client->socket, response, strlen(response), 0);

    // On évite les fuites de mémoire
    free(response);
}

// Démarrage de la socket serveur
int initServerSocket(struct sockaddr_in *adresse)
{
    int fdsocket = initSocket(adresse);

    // Attachement de la socket sur le port et l'adresse IP
    printf("[INFO - SERVER] Attachement de la socket sur le port %i\n", PORT);
    if (bind(fdsocket, (struct sockaddr *)adresse, sizeof(*adresse)) != 0)
    {
        printf("[ERROR - SERVER] Echec d'attachement: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Passage en écoute de la socket
    printf("[INFO - SERVER] Mise en écoute de la socket\n");
    if (listen(fdsocket, BACKLOG) != 0)
    {
        printf("[ERROR - SERVER] Echec de la mise en écoute: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Passage en mode non bloquant
    fcntl(fdsocket, F_SETFL, O_NONBLOCK);

    return fdsocket;
}

// Attente de connexion d'un client
int waitForClient(int *serverSocket)
{
    // Descripteur de socket
    int clientSocket;
    // Structure contenant l'adresse du client
    struct sockaddr_in clientAdresse;
    int addrLen = sizeof(clientAdresse);

    if ((clientSocket = accept(*serverSocket, (struct sockaddr *)&clientAdresse, (socklen_t *)&addrLen)) != -1)
    {
        // Convertion de l'IP en texte
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAdresse.sin_addr), ip, INET_ADDRSTRLEN);
        printf("[INFO - SERVER] Connexion de %s:%i\n", ip, clientAdresse.sin_port);

        // Paramètrage de la socket (mode non bloquant)
        int opt = 1;
        setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(1));
    }
    return clientSocket;
}