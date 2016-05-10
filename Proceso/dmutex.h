#ifndef DMUTEX_H
#define DMUTEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>

#define HOST "localhost"

#define MSG 0
#define LOCK 1
#define OK 2

#define MAX_SIZE 256

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

typedef struct info_sckt
{
  int sckt;
  SOCKADDR_IN sckaddr;
}INFO_SCKT;

typedef struct peer_sckt
{
  char id[80];
  int port;
}PEER_SCKT;

typedef struct udp_msg
{
  int op;
  char idLock[80];
  int *lclk;
}UDP_MSG;

typedef struct mutex
{
  char id[80];
  bool req; //estoy interesado en entrar
  int *req_lclk;
  bool inside;//estoy dentro
  int ok;
  int n_waiting;
  char (*waiting)[80];
}MUTEX;

/* Abre socket y guarda información del socket en el parámetro de entrada */
int open_udp(INFO_SCKT *info);
/* Almacena el proceso y su puerto en nuestra lista */
int store_peer_sckt(const char *proc, const int port);
/* Crea el reloj lógico */
int init_lclk(void);
/* Imprime el reloj lógico */
void print_lclk(void);
/* Combina los relojes lógicos, el local y el del mensaje */
void update_lclk(const int *r_lclk);
/* Serializa el mensaje UDP */
int serialize(const UDP_MSG *msg, unsigned char **buf);
/* Deserializa el mensaje UDP */
UDP_MSG *deserialize(const unsigned char *buf, const size_t bufSz);
/* Obtiene el nombre de un proceso dado un puerto */
int process_name(char *name, const int port);
/* Envia mensaje a destino indicado por parámetro */
int send_message(INFO_SCKT *info, const char *to);
/* Recibe un mensaje UDP */
UDP_MSG *receive_message(const INFO_SCKT *info, char *pname);
/* Obtiene el puerto de un id dado */
int getPort(const char *id);
/* Obtiene el índice de proceso de un id dado */
int getPeerIndex(const char *idPeer);
/* Obtiene el índice de cerrojo de un id de cerrojo dado */
int getLockIndex(const char *idLock);
/* Devuelve 0 si el reloj del argumento 1 es mayor que el del argumento 2 */
int amIOlder(const int *reqLClk, const char *id);
/* Añade un cerrojo a la lista de cerrojos */
int add_lock(const INFO_SCKT *info, const char *id);
/* Elimina un cerrojo de la lista de cerrojos */
int remove_lock(const char *id);
/* Añade un proceso a la lista de espera de un cerrojo */
int addToQueue(const char *idLock, const char* idPeer);
/* Envía las peticiones de OK a la cola del cerrojo y elimina el cerrojo */
int unlock(const INFO_SCKT *info, const char *idLock);
/* Envía OK al destinatario en caso de tener preferencia ante cerrojo */
int sendOkLockRequest(const INFO_SCKT *info, const char *idPeer, const char *idLock);

#endif
