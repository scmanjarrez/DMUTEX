#ifndef _PROCESOS_H_
#define _PROCESOS_H_

#include <stdio.h>
#include "fichero.h"

struct datos_proceso_st
{
  char* ID;
  int   pid;
  int /* FILE* */ in;
  char* init;
};

struct procesos_st
{
  int                       num_procesos;
  struct datos_proceso_st** procesos;
  FILE*                     out;
};

extern int InicializarProcesos(struct procesos_st* procesos,
			       struct fichero_st*  fichero);

extern void STARTProcesos(struct procesos_st* procesos);

extern void ProcesarFichero(struct procesos_st* procesos,
			    struct fichero_st*  fichero);

extern void FinalizarProcesos(struct procesos_st* procesos);
#endif
