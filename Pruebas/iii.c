#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  char (*prueba)[20];
  prueba=malloc(sizeof(char[20]));

  char pp[20];

  strcpy(pp, "holamundo");
  strcpy(*prueba, "mundohola");
  
  printf("pp=%s , tam=%i , *prueba=%s , tam=%i \n", pp, strlen(pp), *prueba,strlen(*prueba));
  free(prueba);
  return 0;
}
