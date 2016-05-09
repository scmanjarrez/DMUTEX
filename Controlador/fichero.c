#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "fichero.h"

#define COMMENT    '#'
#define CHUNK_SIZE 10

static int   lineNo;
static char* fileName;

static int NroProceso(char* proceso, struct fichero_st* fichero)
{
  int i;

  for(i=0;i<fichero->num_procesos;i++)
    if(!strcmp(proceso,fichero->procesos[i]))
      return i;

  fichero->procesos[fichero->num_procesos]=strdup(proceso);
  return fichero->num_procesos++;
}

static struct orden_st* Orden(struct fichero_st* fichero, char* line)
{
  char proc[25],action[60],*aux; 
  struct orden_st* ord=(struct orden_st*)malloc(sizeof(struct orden_st));

  if(line[0]==COMMENT)
    {
      ord->tipo=NULL;
      ord->argument=strdup(line+2);
      return ord;
    }
    
  if((sscanf(line,"%[^:]: %[^\n]",proc,action)!=2))
    {
      free(ord);
      fprintf(stderr,
	      "%s:%d Error de sintaxis 'proceso: acción'\n",
	      fileName,lineNo);
      return NULL;
    }

  ord->proceso=NroProceso(proc,fichero);

  aux=strchr(action,' ');
  if(aux)
    {
      *aux++='\0';
      ord->argument=strdup(aux);
    }
  else
    ord->argument=NULL;
  
  ord->tipo=strdup(action);

  return ord;
}

static int InsertarOrden(struct fichero_st* fichero,
			 struct orden_st* ord,
			 int tam)
{
  if(tam==0)
    {
      fichero->ordenes=(struct orden_st**)
	malloc(sizeof(struct orden_st*)*CHUNK_SIZE);
      bzero((void*)fichero->ordenes,
	    sizeof(struct orden_st*)*CHUNK_SIZE);
      tam=CHUNK_SIZE;
    }

  if(tam==fichero->num_ordenes)
    {	    
      fichero->ordenes=(struct orden_st**)
	realloc(fichero->ordenes,
		sizeof(struct orden_st*)*(tam+CHUNK_SIZE));
      bzero((void*)(fichero->ordenes+tam),
	    sizeof(struct orden_st*)*CHUNK_SIZE);
      tam+=CHUNK_SIZE;
    }
  fichero->ordenes[fichero->num_ordenes]=ord;

  return tam;
}
			  

int LeerFichero(struct fichero_st* fichero,
		char*        nombre_fichero)
{
  FILE* f;
  char line[80];
  struct orden_st* ord;
  int tam;

  fileName=nombre_fichero;

  if(!(f=fopen(nombre_fichero,"r")))
    {
      fprintf(stderr,"Error al abrir el archivo\n");
      perror("fopen");
      return 1;
    }

  for(tam=0,fichero->num_ordenes=0,fichero->num_procesos=0,lineNo=1;
      fgets(line,80,f);
      fichero->num_ordenes++,lineNo++)
    {
      ord=Orden(fichero,line);
      tam=InsertarOrden(fichero,ord,tam);
    }
  return 0;
}
