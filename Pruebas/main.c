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
  int ok;
  bool req; //estoy interesado en entrar
  int *req_lclk;
  bool inside;//estoy dentro
  int n_waiting;
  char (*waiting)[80];
}MUTEX;

int open_udp(INFO_SCKT *info);
int store_peer_sckt(const char *proc, const int port);
int init_lclk(void);
void print_lclk(void);
void update_lclk(const int *r_lclk);
int serialize(const UDP_MSG *msg, unsigned char **buf);
UDP_MSG *deserialize(const unsigned char *buf, const size_t bufSz);
int process_name(char *name, const int port);
int send_message(INFO_SCKT *info, const char *to);
UDP_MSG *receive_message(const INFO_SCKT *info, char *pname);
int getPort(const char *id);
int getPeerIndex(const char *idPeer);
int getLockIndex(const char *idLock);
int add_waiting_to_lock(const char *idLock, const char* idPeer);
int amIOlder(const int *myLClk, const int *hisLClk, const char *id);//0 si soy más antiguo que el otro
int add_lock(const INFO_SCKT *info, const char *id);
int remove_lock(const char *id);

int udp_port;
PEER_SCKT *peers;
int n_peers;
int *lclk;
int *past_lclk;
int myIndex;
char crit_sect[80];
MUTEX *locks;
int n_locks;

int main(int argc, char* argv[])
{
  int port;
  char line[80],proc[80];

  if(argc<2)
    {
      fprintf(stderr,"Uso: proceso <ID>\n");
      return 1;
    }

  /* Establece el modo buffer de entrada/salida a línea */
  setvbuf(stdout,(char*)malloc(sizeof(char)*80),_IOLBF,80); 
  setvbuf(stdin,(char*)malloc(sizeof(char)*80),_IOLBF,80); 

  INFO_SCKT info;
  if(open_udp(&info)==-1)
    {
      fprintf(stderr, "Error al crear el socket UDP\n");
      return EXIT_FAILURE;
    }

  udp_port=ntohs(info.sckaddr.sin_port);
  
  fprintf(stdout,"%s: %d\n",argv[1],udp_port);

  if((peers = malloc(sizeof(PEER_SCKT)))==NULL)
    {
      fprintf(stderr, "Error al crear la lista de procesos\n");
      return EXIT_FAILURE;
    }

  if((locks = malloc(sizeof(MUTEX)))==NULL)
    {
      fprintf(stderr, "Error al crear la lista de cerrojos\n");
      return EXIT_FAILURE;
    }

    if((locks->waiting = malloc(sizeof(PEER_SCKT)))==NULL)
    {
      fprintf(stderr, "Error al crear la lista de espera en cerrojo\n");
      return EXIT_FAILURE;
    }

  
  for(;fgets(line,80,stdin);)
    {
      if(!strcmp(line,"START\n"))
	break;            

      sscanf(line,"%[^:]: %d",proc,&port);
      if(!strcmp(proc,argv[1]))
	{
	  myIndex = n_peers;	  
	}

      if(store_peer_sckt(proc, port)==-1)
	{
	  fprintf(stderr, "Error al almacenar información de procesos\n");
	  free(peers);
	  return EXIT_FAILURE;
	}
    }

  /* Dejamos el tamaño final */
  if((peers=realloc(peers, (n_peers)*sizeof(PEER_SCKT)))==NULL)
    {
      fprintf(stderr, "Error al dejar el tamaño final de la lista de procesos\n");
      free(peers);
      return EXIT_FAILURE;
    }
  
  /* Inicializar Reloj */
  if(init_lclk()==-1)
    {
      fprintf(stderr, "Error al inicializar los relojes lógicos\n");
      free(peers);
      return EXIT_FAILURE;
    }
  
  /* Procesar Acciones */
  char id_sec[80];
  char action[80];
  
  for(;fgets(line,80,stdin);)
    {
      if(!strcmp(line,"EVENT\n"))
	{
	  lclk[myIndex]++;
	  printf("%s: TICK\n", peers[myIndex].id);
	  continue;
	}

      if(!strcmp(line,"GETCLOCK\n"))
	{
	  print_lclk();
	  continue;
	}       

      if(!strcmp(line,"RECEIVE\n"))
	{
	  UDP_MSG *msg;
	  char pname[80];
	  if((msg=receive_message(&info, pname))==NULL)
	    {
	      fprintf(stderr, "Error al recibir mensaje\n");
	      free(msg);
	      free(lclk);
	      free(peers);
	      return EXIT_FAILURE;
	    }
	  switch(msg->op)
	    {
	    case MSG:
	      printf("%s: RECEIVE(MSG,%s)\n", peers[myIndex].id, pname);
	      update_lclk(msg->lclk);

	      lclk[myIndex]++;
	      printf("%s: TICK\n", peers[myIndex].id);
	      break;
	      case LOCK:
	        break;
	      case OK:
		break;

	    }
	}       

      if(!strcmp(line,"FINISH\n"))
	{
	  printf("%s: FINISH[%i]\n", peers[myIndex].id, getpid());
	  break;
	}
      
      sscanf(line,"%s %s",action,id_sec);
      
      if(!strcmp(action,"MESSAGETO"))
	{
	  if(send_message(&info, id_sec)==-1)
	    {
	      free(lclk);
	      free(peers);
	      return EXIT_FAILURE;
	    }
	}       

      if(!strcmp(action,"LOCK"))
      	{
      	}

      if(!strcmp(action,"UNLOCK"))
      	{
      	}

    }
  free(lclk);
  free(peers);
  return EXIT_SUCCESS;
}

int store_peer_sckt(const char *proc, const int port)
{
  strcpy(peers[n_peers].id, proc);
  peers[n_peers++].port = port;
  if((peers=realloc(peers, (1+n_peers)*sizeof(PEER_SCKT)))==NULL)
    {
      perror("Error");
      return -1;
    }
  
  return EXIT_SUCCESS;
}

int open_udp(INFO_SCKT *info)
{

  if((info->sckt=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1){
    perror("Error");
    return -1;
  }

  bzero((char*)&(info->sckaddr), sizeof(SOCKADDR_IN));                                            
  
  info->sckaddr.sin_family = AF_INET;                                
  info->sckaddr.sin_addr.s_addr=INADDR_ANY;
  info->sckaddr.sin_port = htons(0);

  if(bind(info->sckt, (struct sockaddr*)&(info->sckaddr), sizeof(SOCKADDR_IN))==-1){
    perror("Error");
    close(info->sckt);
    return -1;
  }
  
  int tam_dir=sizeof(SOCKADDR_IN);
  
  if(getsockname(info->sckt, (struct sockaddr*)&(info->sckaddr), (socklen_t*) &tam_dir)==-1){
    perror("Error");
    close(info->sckt);
    return -1;
  }

  return EXIT_SUCCESS;
}

int init_lclk(void)
{

  if((lclk=calloc(n_peers, sizeof(int)))==NULL)
    {
      perror("Error");
      return -1;
    }

    if((past_lclk=calloc(n_peers, sizeof(int)))==NULL)
    {
      perror("Error");
      return -1;
    }

  return EXIT_SUCCESS;
}

void print_lclk(void)
{
  int i;
  printf("%s: LC[", peers[myIndex].id);
  for(i=0; i<n_peers; i++)
    {
      if(i==n_peers-1)
	printf("%i]\n", lclk[i]);
      else
	printf("%i,", lclk[i]);
    }
}

void update_lclk(const int *r_lclk)
{
  int i;
  for(i=0; i<n_peers; i++)
    lclk[i] = lclk[i]>r_lclk[i]? lclk[i]:r_lclk[i];
}

int serialize(const UDP_MSG *msg, unsigned char **buf)
{
  /* Tipo_Mensaje + 80 id + n*proceso_clk */
  int msg_sz = sizeof(uint32_t)+80+n_peers*sizeof(uint32_t);

  if((*buf = calloc(1, msg_sz))==NULL)
    {
      perror("Error");
      return -1;
    }

  size_t offset = 0;
  unsigned int tmp;
  int i;
  // Tipo_Mensaje
  tmp = htonl(msg->op);
  memcpy(*buf + offset, &tmp, sizeof(tmp));
  offset += sizeof(tmp);

  // idLock
  memcpy(*buf + offset, msg->idLock, 80);
  offset += 80;

  for(i=0; i<n_peers; i++)
    {
      // i-proceso_clk
      tmp = htonl(lclk[i]);
      memcpy(*buf + offset, &tmp, sizeof(tmp));
      offset += sizeof(tmp);  
    }
    
  return msg_sz;
}

UDP_MSG *deserialize(const unsigned char *buf, const size_t bufSz)
{
  /* 4B Tipo Mensaje, 80 char, 4B 1 único proceso */
  static const size_t MIN_BUF_SZ = 88;

  UDP_MSG  *msg;

  if (buf && bufSz < MIN_BUF_SZ)
    {
      fprintf(stderr, "El tamaño del buffer es menor que el mínimo\n");
      return NULL;
    }

  if((msg = calloc(1, sizeof(UDP_MSG)))==NULL)
    {
      perror("Error");
      return NULL;
    }

  size_t tmp = 0, offset = 0;

  // obtenemos tipo mensaje
  memcpy(&tmp, buf + offset, sizeof(tmp));
  tmp = ntohl(tmp);
  memcpy(&msg->op, &tmp, sizeof(uint32_t));
  offset  += sizeof(uint32_t);

  // obtenemos el idLock
  memcpy(&msg->idLock, buf + offset, 80);
  offset  += 80;

  int i;

  if((msg->lclk=calloc(n_peers, sizeof(uint32_t)))==NULL)
    {
      perror("Error");
      free(msg);
      return NULL;
    }

  for(i=0; i<n_peers; i++)
    {
      // obtenemos i-proceso_clk
      memcpy(&tmp, buf + offset, sizeof(tmp));
      tmp = ntohl(tmp);
      memcpy(&msg->lclk[i], &tmp, sizeof(uint32_t));
      offset  += sizeof(uint32_t);
    }

  return msg;
}

int process_name(char *name, const int port)
{
  int i;
  bool found=false;

  for(i=0; i<n_peers && !found; i++)
    {
      if(peers[i].port == port)
	{
	  found=true;
	  strcpy(name, peers[i].id);
	  return EXIT_SUCCESS;
	}
    }

  return -1;
}

int send_message(INFO_SCKT *info, const char *to)
{
  /* Se genera un evento */
  lclk[myIndex]++;
  printf("%s: TICK\n", peers[myIndex].id);
  
  /* Y se envia el reloj lógico */
  UDP_MSG msg;
  bzero((char*)&msg, sizeof(UDP_MSG));
  
  msg.op=MSG;
  int port;
  if((port=getPort(to))==-1)
    {
      fprintf(stderr,"No se ha encontrado el destinatario indicado\n");
      return -1;
    }

  SOCKADDR_IN receiver;
  struct hostent *netdb;
  netdb = gethostbyname(HOST);                                           

  receiver.sin_family = AF_INET;                                
  memcpy(&(receiver.sin_addr), netdb->h_addr, netdb->h_length);
  receiver.sin_port = htons(port);


  int msg_sz;
  unsigned char *buf = 0;
  if((msg_sz = serialize(&msg, &buf))==-1)
    {
      fprintf(stderr, "Error al serializar mensaje(MSG)\n");
      return -1;
    }

  int tam_d=sizeof(SOCKADDR_IN);
  
  if((sendto(info->sckt, buf, msg_sz,0, (struct sockaddr*)&receiver,tam_d))==-1)
    {
      fprintf(stderr, "Error al enviar mensaje(MSG) a %s\n", to);
      return -1;
    }
  printf("%s: SEND(MSG,%s)\n",peers[myIndex].id, to);
 
  return EXIT_SUCCESS;
}

UDP_MSG *receive_message(const INFO_SCKT *info, char *pname)
{
  SOCKADDR_IN rec;
  unsigned char buff[MAX_SIZE] = {0};
  UDP_MSG *msg;
  int tam = sizeof(SOCKADDR_IN);
  int msg_sz;
	  
  if((msg_sz=recvfrom(info->sckt, buff, MAX_SIZE, 0, (SOCKADDR*)&rec, (socklen_t*)&tam))==-1)
    {
      perror("Error");
      return NULL;
    }

  if((msg=deserialize(buff, msg_sz))==NULL)
    {
      fprintf(stderr, "Error al deserializar el mensaje\n");
      return NULL;
    }
	  
  if((process_name(pname, ntohs(rec.sin_port)))==-1)
    {
      fprintf(stderr, "No ha sido posible identificar el emisor del mensaje\n");
      return NULL;
    }
  
  return msg;
}

int getPort(const char *id)
{
  int i;
  bool found=false;
  for(i=0; i<n_peers && !found; i++)
    {
      if(!strcmp(id, peers[i].id))
	{
	  found=true;
	  return peers[i].port;
	}
    }
  
  return -1;
}

/* int send_lock_request(const INFO_SCKT *info) */
/* { */
/*   /\* Se genera un evento *\/ */
/*   lclk[myIndex]++; */
/*   printf("%s: TICK\n", peers[myIndex].id); */

/*   int i; */
/*   for(i=0; i<n_peers;i++) */
/*     past_lclk[i] = lclk[i]; */
 
/*   /\* Y se envia el reloj lógico a todos los procesos *\/ */
/*   UDP_MSG msg; */
/*   bzero((char*)&msg, sizeof(UDP_MSG)); */
  
/*   msg.op=LOCK; */

/*   SOCKADDR_IN receiver; */
/*   struct hostent *netdb; */
/*   netdb = gethostbyname(HOST); */

/*   receiver.sin_family = AF_INET; */
/*   memcpy(&(receiver.sin_addr), netdb->h_addr, netdb->h_length); */

/*   for(i=0; i<n_peers;i++) */
/*     { */
/*       if(i!=myIndex) */
/* 	{ */
/* 	  receiver.sin_port = htons(peers[i].port); */

/* 	  int msg_sz; */
/* 	  unsigned char *buf = 0; */
/* 	  if((msg_sz = serialize(MSG, &buf))==-1) */
/* 	    { */
/* 	      fprintf(stderr, "Error al serializar mensaje(LOCK)\n"); */
/* 	      return -1; */
/* 	    } */
	  
/* 	  int tam_d=sizeof(SOCKADDR_IN); */
  
/* 	  if((sendto(info->sckt, buf, msg_sz,0, (struct sockaddr*)&receiver,tam_d))==-1) */
/* 	    { */
/* 	      fprintf(stderr, "Error al enviar mensaje(LOCK) a %s\n", peers[i].id); */
/* 	      return -1; */
/* 	    } */
/* 	  printf("%s: SEND(LOCK,%s)\n",peers[myIndex].id, peers[i].id); */
	  
/* 	} */

/*     } */
 
/*   return EXIT_SUCCESS; */
/* } */

int unlock(const INFO_SCKT *info, const char *idLock)
{
  int lockIndex;
  
  if ((lockIndex=getLockIndex(idLock))==-1)
    {
      fprintf(stderr, "No se ha encontrado el cerrojo con el id \"%s\"\n", idLock);
      return -1;
    }

  UDP_MSG msg;
  bzero((char*)&msg, sizeof(UDP_MSG));
  
  msg.op=OK;

  SOCKADDR_IN receiver;
  struct hostent *netdb;
  netdb = gethostbyname(HOST);

  receiver.sin_family = AF_INET;
  memcpy(&(receiver.sin_addr), netdb->h_addr, netdb->h_length);

  if(locks[lockIndex].n_waiting)
    {
      /* Se genera un evento */
      lclk[myIndex]++;
      printf("%s: TICK\n", peers[myIndex].id);

      int i, peerIndex;
      for(i=0; i<locks[lockIndex].n_waiting;i++)
	{
	  if ((peerIndex=getPeerIndex(locks[lockIndex].waiting[i]))==-1)
	    {
	      fprintf(stderr, "No se ha encontrado el destinatario con id \"%s\"\n", locks[lockIndex].waiting[i]);
	      return -1;
	    }

	  receiver.sin_port = htons(peers[peerIndex].port);
	  
	  int msg_sz;
	  unsigned char *buf = 0;
	  if((msg_sz = serialize(MSG, &buf))==-1)
	    {
	      fprintf(stderr, "Error al serializar mensaje(OK)\n");
	      return -1;
	    }
      
	  int tam_d=sizeof(SOCKADDR_IN);
  
	  if((sendto(info->sckt, buf, msg_sz,0, (struct sockaddr*)&receiver,tam_d))==-1)
	    {
	      fprintf(stderr, "Error al enviar mensaje(OK) a %s\n", locks[lockIndex].waiting[i]);
	      return -1;
	    }
	  printf("%s: SEND(OK,%s)\n",peers[myIndex].id, locks[lockIndex].waiting[i]);
	  
	}
    }
    
  return 0;
}

int addToQueue(const char *idLock, const char* idPeer)
{
  int lockIndex;
  
  if ((lockIndex=getLockIndex(idLock))==-1)
    {
      fprintf(stderr, "No se ha encontrado el cerrojo con el id \"%s\"\n", idLock);
      return -1;
    }

  if (getPeerIndex(idPeer)==-1)
    {
      fprintf(stderr, "No se ha encontrado el destinatario con id \"%s\"\n", idPeer);
      return -1;
    }
  
  strcpy(locks[lockIndex].waiting[locks[lockIndex].n_waiting++], idPeer);
  if((locks[lockIndex].waiting = realloc(locks[lockIndex].waiting, (1+locks[lockIndex].n_waiting)*sizeof(char[80])))==NULL)
    {
      fprintf(stderr, "Error al reservar memoria en cola para cerrojo \"%s\"\n", idLock);
      return -1;
    }
  return 0;
}

int getLockIndex(const char *idLock)
{
  int i;
  bool found=false;
  for (i=0; i < n_locks && !found; i++)
    {
      if (!strcmp(locks[i].id, idLock))
	{
	  return i;
	}
    }
  return -1;
}

int getPeerIndex(const char *idPeer)
{
  int i;
  bool found=false;
  for (i=0; i < n_peers && !found; i++)
    {
      if (!strcmp(peers[i].id, idPeer))
	{
	  return i;
	}
    }
  return -1;
}

int amIOlder(const int *myLClk, const int *hisLClk, const char *id)
{
  int i;
  for (i = 0; i<n_peers; i++)
    {
      if(myLClk[i]>hisLClk[i])
	{
	  return 0;
	}    
  }

  int pIndex;
  if((pIndex=getPeerIndex(id))==-1)
    {
      fprintf(stderr, "El proceso con id \"%s\" no se ha encontrado\n", id);
      return -1;
    }
  return myIndex>pIndex? 0:1;
}

int remove_lock(const char *id)
{
  int lockIndex;

  if((lockIndex=getLockIndex(id))==-1)
    {
      fprintf(stderr, "No se ha encontrado el lock con id \"%s\"\n", id);
      return -1;
    }

  MUTEX *temp = malloc((n_locks)*sizeof(MUTEX));
  
  if(!lockIndex)
    {
      memmove(temp, locks+1, (n_locks-1)*sizeof(MUTEX));
    }
  else
    {
      memmove(temp, locks, (lockIndex)*sizeof(MUTEX));
      memmove(temp+lockIndex, locks+lockIndex+1, (n_locks-lockIndex-1)*sizeof(MUTEX));
    }
  
  free(locks[lockIndex].waiting);
  free(locks);
  n_locks--;
  locks = temp;

  return 0;
}

int add_lock(const INFO_SCKT *info, const char *id)
{
  if ((getLockIndex(id)!=-1))
    {
      fprintf(stderr, "Ya se ha hecho una petición del lock %s anteriormente\n", id);
      return -1;
    }

  /* Lo añadimos a la lista de locks */
  strcpy(locks[n_locks].id, id);
  locks[n_locks].ok = 0;
  locks[n_locks].req = true;
  locks[n_locks].inside = false;
  locks[n_locks].n_waiting = 0;
  if ((locks[n_locks].req_lclk = malloc(n_peers*sizeof(int)))==NULL)
    {
      fprintf(stderr, "Error al reservar memoria para reloj lógico antiguo de lock \"%s\"\n", id);
      return -1;
    }

  int i;
  for(i=0; i<n_peers;i++)
    locks[n_locks].req_lclk[i] = lclk[i];


  if ((locks[n_locks++].waiting = malloc(sizeof(char[80])))==NULL)
    {
      fprintf(stderr, "Error al reservar memoria para lista de espera de lock \"%s\"\n", id);
      return -1;
    }
  if((locks = realloc(locks, (1+n_locks)*sizeof(MUTEX)))==NULL)
    {
      fprintf(stderr, "Error al reservar espacio extra para nuevo lock\n");
      return -1;
    }

  /* Se genera un evento */
  lclk[myIndex]++;
  printf("%s: TICK\n", peers[myIndex].id);
    
  /* Y se envia el reloj lógico a todos los procesos */
  UDP_MSG msg;
  bzero((char*)&msg, sizeof(UDP_MSG));
  
  msg.op=LOCK;
  strcpy(msg.idLock, id);

  SOCKADDR_IN receiver;
  struct hostent *netdb;
  netdb = gethostbyname(HOST);

  receiver.sin_family = AF_INET;
  memcpy(&(receiver.sin_addr), netdb->h_addr, netdb->h_length);

  for(i=0; i<n_peers;i++)
    {
      if(i!=myIndex)
	{
	  receiver.sin_port = htons(peers[i].port);

	  int msg_sz;
	  unsigned char *buf = 0;
	  if((msg_sz = serialize(&msg, &buf))==-1)
	    {
	      fprintf(stderr, "Error al serializar mensaje(LOCK)\n");
	      return -1;
	    }
	  
	  int tam_d=sizeof(SOCKADDR_IN);
  
	  if((sendto(info->sckt, buf, msg_sz,0, (struct sockaddr*)&receiver,tam_d))==-1)
	    {
	      fprintf(stderr, "Error al enviar mensaje(LOCK) a %s\n", peers[i].id);
	      return -1;
	    }
	  printf("%s: SEND(LOCK,%s)\n",peers[myIndex].id, peers[i].id);
	  
	}

    }
    
  return 0;
}
