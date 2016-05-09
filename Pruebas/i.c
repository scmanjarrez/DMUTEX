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

int main()
{

  INFO_SCKT info;
  if(open_udp(&info)==EXIT_FAILURE)
    {
      fprintf(stderr, "Error al crear el socket UDP\n");
      return EXIT_FAILURE;
    }
  printf("puerto = %i\n",ntohs(info.sckaddr.sin_port));
  SOCKADDR_IN rec;
  int tam_d=sizeof(SOCKADDR_IN);
  char algo[50];
  
  recvfrom(info.sckt, &algo, 50, 0, (struct sockaddr *)&rec, (socklen_t *)&tam_d);

  printf("mensaje = %s\n", algo);
  

  return EXIT_SUCCESS;
}

int open_udp(INFO_SCKT *info)
{

  if((info->sckt=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1){
    perror("Error:");
    return EXIT_FAILURE;
  }

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
