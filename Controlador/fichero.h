#ifndef _FICHERO_H_
#define _FICHERO_H_

#define MAX_PROCESOS 20

struct orden_st
{
  int   proceso;
  char* tipo;
  char* argument;
};

struct fichero_st
{
  int   num_procesos;
  char* procesos[MAX_PROCESOS];

  int               num_ordenes;
  struct orden_st** ordenes;
};

extern int LeerFichero(struct fichero_st* fichero,
		       char*              nombre_fichero);



#endif
