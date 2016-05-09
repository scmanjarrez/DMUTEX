#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#define HOST "localhost"

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct info_sckt
{
  int sckt;
  struct sockaddr_in sckaddr;
}INFO_SCKT;

int open_udp(INFO_SCKT *info);

int main(int argc, char *argv[])
{

  INFO_SCKT info;
  if(open_udp(&info)==EXIT_FAILURE)
    {
      fprintf(stderr, "Error al crear el socket UDP\n");
      return EXIT_FAILURE;
    }

  int puerto=atoi(argv[1]);

  SOCKADDR_IN sender;
  struct hostent *netdb;
  netdb = gethostbyname(HOST);                                           

  sender.sin_family = AF_INET;                                
  memcpy(&(sender.sin_addr), netdb->h_addr, netdb->h_length);
  sender.sin_port = htons(puerto);

  char *hola="holamundo";
  int tam_h=strlen(hola);
  int tam_d=sizeof(SOCKADDR_IN);
  
  sendto(info.sckt, hola, tam_h,0, (struct sockaddr*)&sender,tam_d);
  return EXIT_SUCCESS;
}

int open_udp(INFO_SCKT *info)
{

  if((info->sckt=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1){
    perror("Error:");
    return EXIT_FAILURE;
  }

  struct hostent *netdb;
  netdb = gethostbyname(HOST);                                           


  bzero((char*)&(info->sckaddr), sizeof(struct sockaddr_in));                                            
  
  info->sckaddr.sin_family = AF_INET;                                
  info->sckaddr.sin_addr.s_addr=INADDR_ANY;
  info->sckaddr.sin_port = htons(0);

  if(bind(info->sckt, (struct sockaddr*)&(info->sckaddr), sizeof(struct sockaddr_in))==-1){
    perror("Error:");
    close(info->sckt);
    return EXIT_FAILURE;
  }
  int tam_dir=sizeof(struct sockaddr_in);
  if(getsockname(info->sckt, (struct sockaddr*)&(info->sckaddr), (socklen_t*) &tam_dir)==-1){
    perror("Error:");
    close(info->sckt);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
