#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include "logger.h"

long log_tick=DEFAULT_LOG_TICK;

static pthread_t tid;
static pthread_mutex_t log_mutex;

static char*   action=NULL;
static char*** log_messages;
static int*    num_messages,num_procesos;
static struct procesos_st* procesos;

struct timespec log_delay;

void* Logger(void* arg)
{
  char line[80],id[80],msg[80];
  int i;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  while(1)
    {
      fgets(line,79,procesos->out);
      line[78]='\n';
      line[79]='\0';

      sscanf(line,"%[^:]: %[^\n]",id,msg);
      
      pthread_mutex_lock(&log_mutex);
      for(i=0;i<procesos->num_procesos;i++)
	if(!strcmp(procesos->procesos[i]->ID,id))
	  {	    
	    if(log_messages[i]==NULL)
	      log_messages[i]=(char**)malloc(sizeof(char*));
	    else	      
	      log_messages[i]=
		(char**)realloc(log_messages[i],
				sizeof(char*)*(num_messages[i]+1));
	    log_messages[i][num_messages[i]++]=strdup(msg);	    
	    break;
	  }
      if(i==procesos->num_procesos)
	fprintf(stderr,"Mensaje no reconocido: %s",line);
      pthread_mutex_unlock(&log_mutex);
    }

  return NULL;
}

void IniciarLogger(struct procesos_st* proc)
{
  pthread_mutex_init(&log_mutex,NULL);

  procesos=proc;

  num_procesos=procesos->num_procesos;
  log_delay.tv_sec=0;
  log_delay.tv_nsec=1000*log_tick;

  log_messages=(char***)malloc(sizeof(char**)*num_procesos);
  bzero((void*)log_messages,sizeof(char**)*num_procesos);
  num_messages=(int*)malloc(sizeof(int)*num_procesos);
  bzero((void*)num_messages,sizeof(int)*num_procesos);

  pthread_create(&tid,NULL,Logger,NULL);
}


void InicioLogPoint(char* linea)
{
  pthread_mutex_lock(&log_mutex);  

  action=strdup(linea);

  pthread_mutex_unlock(&log_mutex);
}

void FinLogPoint()
{
  int i,j;

  if(log_tick)
    nanosleep(&log_delay,NULL);

  pthread_mutex_lock(&log_mutex);  

  if(action)
    {
      fprintf(stdout,"%s->",action);
      free(action);
    }
  
  for(i=0;i<num_procesos;i++)
    {
      fprintf(stdout," %s{",procesos->procesos[i]->ID);
      if(num_messages[i])
	{
	  for(j=0;j<num_messages[i];j++)
	    {
	      fprintf(stdout,"%s%c",
		      log_messages[i][j],
		      (j==num_messages[i]-1)?'}':'|');
	      free(log_messages[i][j]);
	    }
	  num_messages[i]=0;
	  free(log_messages[i]);
	  log_messages[i]=NULL;
	}
      else
	fprintf(stdout,"--}");
    }

  fprintf(stdout,"\n");
  pthread_mutex_unlock(&log_mutex);
}


void FinalizarLogger()
{
  pthread_cancel(tid);
  pthread_join(tid,NULL);
}
