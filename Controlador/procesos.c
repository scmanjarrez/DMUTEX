#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "procesos.h"
#include "logger.h"

#define CHUNK_SIZE 3

static struct datos_proceso_st* ArrancarProceso(char* procID, int out)
{
  struct datos_proceso_st* proc;
  int    in[2];

  proc=(struct datos_proceso_st*)malloc(sizeof(struct datos_proceso_st));
  
  pipe(in);
  switch(proc->pid=fork())
    {
    case -1: 
      fprintf(stderr,
	      "Ha sido imposible arrancar el proceso %s\n", 
	      procID);
      perror("fork");
      free(proc);
      close(in[0]); 
      close(in[1]); 
      return NULL;
    case 0: /* HIJO */
      close(0); dup(in[0]);  close(in[0]);  close(in[1]);
      close(1); dup(out);    close(out); 
      execl("./proceso", "./proceso", procID, NULL);
      fprintf(stderr,
	      "Ha sido imposible ejecutar el proceso %s\n", 
	      procID);
      perror("execl");
      exit(1);
    default: /* PADRE */
      close(in[0]);  
      proc->in=in[1];/* fdopen(in[1],"w");     */
/*       setvbuf(proc->in,(char*)malloc(sizeof(char)*80),_IOLBF,80);  */
    }

  proc->ID=strdup(procID);

  return proc;
}

static int InsertarProceso(struct procesos_st* procesos,
			   struct datos_proceso_st* proc,
			   int tam, int usd)
{
  if(tam==0)
    {
      procesos->procesos=(struct datos_proceso_st**)
	malloc(sizeof(struct datos_proceso_st*)*CHUNK_SIZE);
      bzero((void*)procesos->procesos,
	    sizeof(struct datos_proceso_st*)*CHUNK_SIZE);
      tam=CHUNK_SIZE;
    }

  if(tam==usd)
    {	    
      procesos->procesos=(struct datos_proceso_st**)
	realloc(procesos->procesos,
		sizeof(struct datos_proceso_st*)*(tam+CHUNK_SIZE));
      bzero((void*)(procesos->procesos+tam),
	    sizeof(struct datos_proceso_st*)*CHUNK_SIZE);
      tam+=CHUNK_SIZE;
    }
  procesos->procesos[usd]=proc;

  return tam;
}
	
static void IntercambiarPuertos(int p,
				struct procesos_st* procesos)
{
  int i;

  for(i=0;i<procesos->num_procesos;i++)
    write(procesos->procesos[i]->in,
	  procesos->procesos[p]->init,
	  strlen(procesos->procesos[p]->init));
/*     fprintf(procesos->procesos[i]->in,procesos->procesos[p]->init); */
}
				
		  
int InicializarProcesos(struct procesos_st* procesos,
			struct fichero_st*  fichero)
{
  int i,tam=0,out[2];
  struct datos_proceso_st* proc;
  char line[80];

  procesos->num_procesos=fichero->num_procesos;

  pipe(out);

  procesos->out=fdopen(out[0],"r"); 
  setvbuf(procesos->out,(char*)malloc(sizeof(char)*80),_IOLBF,80); 

  for(i=0;i<fichero->num_procesos;i++)
    {
      proc=ArrancarProceso(fichero->procesos[i],out[1]);
      tam=InsertarProceso(procesos,proc,tam,i);
      fgets(line,80,procesos->out);     
      proc->init=strdup(line);
    }

  close(out[1]);

/*   setbuf(procesos->out,NULL); */

  for(i=0;i<fichero->num_procesos;i++)
    IntercambiarPuertos(i,procesos);

  return 0;
}

void STARTProcesos(struct procesos_st* procesos)
{
  int i;

  for(i=0;i<procesos->num_procesos;i++)
/*     fprintf(procesos->procesos[i]->in,"START\n");*/
    write(procesos->procesos[i]->in,"START\n",strlen("START\n"));
}

void ProcesarFichero(struct procesos_st* procesos,
		     struct fichero_st*  fichero)
{
  char action[80];
  int i;

  IniciarLogger(procesos);

  for(i=0;i<fichero->num_ordenes;i++)
    if(!fichero->ordenes[i])
      continue;
    else if(!fichero->ordenes[i]->tipo)
      fprintf(stdout,"# %s",fichero->ordenes[i]->argument);
    else if(fichero->ordenes[i]->tipo)
      {    
	if(fichero->ordenes[i]->argument)
	  sprintf(action,"%s: [%s %s]",
		  procesos->procesos[fichero->ordenes[i]->proceso]->ID,
		  fichero->ordenes[i]->tipo,
		  fichero->ordenes[i]->argument);	
	else
	  sprintf(action,"%s: [%s]",
		  procesos->procesos[fichero->ordenes[i]->proceso]->ID,
		  fichero->ordenes[i]->tipo);	

	InicioLogPoint(action);

	if(fichero->ordenes[i]->argument)
	  sprintf(action,"%s %s\n",
		  fichero->ordenes[i]->tipo,
		  fichero->ordenes[i]->argument);		
	else
	  sprintf(action,"%s\n",
		  fichero->ordenes[i]->tipo);		

	write(procesos->procesos[fichero->ordenes[i]->proceso]->in,
	      action,strlen(action));
/* 	fprintf(procesos->procesos[fichero->ordenes[i]->proceso]->in,action); */
	/* fflush(procesos->procesos[fichero->ordenes[i]->proceso]->in); */
	FinLogPoint();
      }
  
  FinalizarLogger();
}

void FinalizarProcesos(struct procesos_st* procesos)
{
  int i,ret;
   
  for(i=0;i<procesos->num_procesos;i++)
    {
      /* fprintf(procesos->procesos[i]->in,"FINISH\n");  */
      write(procesos->procesos[i]->in,"FINISH\n",strlen("FINISH\n"));
      waitpid(procesos->procesos[i]->pid,&ret,0);
    }  
}
