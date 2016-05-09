#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <strings.h>
#include "fichero.h"
#include "procesos.h"
#include "logger.h"

static char* end;
static struct fichero_st fichero;
static struct procesos_st procesos;
static struct sigaction hijo_muerto;

void HIJO_MUERTO(int signum, siginfo_t *info, void *val)
{
  int i;
  char* proc="---";
  for(i=0;i<procesos.num_procesos;i++)
    if(info->si_pid==procesos.procesos[i]->pid)
      proc=procesos.procesos[i]->ID;

  fprintf(stdout," Se ha muerto el proceso %d[%s]: ret. value:%d\n",
	  info->si_pid,proc,
	  info->si_status);
}

static void fin_plazo(int s) {
	system("kill -9 `ps -ef | grep ssoo | grep -w ./proceso | awk '{print $2}'` >/dev/null 2>&1");
        exit(1);
}

int main(int argc, char* argv[])
{
  hijo_muerto.sa_sigaction=HIJO_MUERTO;
  hijo_muerto.sa_flags=SA_SIGINFO;

  sigaction(SIGCHLD, &hijo_muerto, NULL);

  signal(SIGALRM, fin_plazo);
  alarm(120);


  if(argc<2)
    {
      fprintf(stderr,
	      "Uso: controlador <fichero> [<tick>]\n"
	      "\t<tick>: Número de microseg. de espera en cada\n"
	      "\t        accion (OPCIONAL).\n"
	      "\t        tick tiene que ser menor que 1 seg.\n");
      return 1;
    }

  if(argc==3)
    {
      log_tick=strtol(argv[2],&end,10);
      if(*end)
	{
	  fprintf(stderr,"Valor de tick '%s' no válido\n",argv[2]);
	  return 2;
	}
    }

  if(LeerFichero(&fichero,argv[1]))
    {
      fprintf(stderr,"Error al leer fichero %s\n",argv[1]);
      return 3;
    }

  if(InicializarProcesos(&procesos,&fichero))
    {
      fprintf(stderr,"Error al inicializar procesos\n");
      return 4;
    }

  STARTProcesos(&procesos);

  ProcesarFichero(&procesos,&fichero);

  hijo_muerto.sa_sigaction=NULL;
  hijo_muerto.sa_flags=SA_NOCLDSTOP;

  sigaction(SIGCHLD, &hijo_muerto, NULL);

  FinalizarProcesos(&procesos);

  return 0;
}
