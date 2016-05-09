#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <pthread.h>
#include "procesos.h"

#define DEFAULT_LOG_TICK 50000

extern long log_tick;

extern void IniciarLogger(struct procesos_st* procesos);
extern void InicioLogPoint(char* linea);
extern void FinLogPoint();
extern void FinalizarLogger();

#endif
