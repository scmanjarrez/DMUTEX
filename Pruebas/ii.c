#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

typedef struct mia
{
  int num;
  struct sockaddr_in addr;
}MIA;
  
int main()
{
// printf("success=%i, error=%i\n", EXIT_SUCCESS, EXIT_FAILURE);
MIA a;
MIA *b=&a;
a.num=5;
b->num=4;

printf("anum=%i, bnum=%i\n", a.num, b->num);
printf("addr &a.num=%p, &a.addr=%p\n", &(a.num), &(a.addr));

printf("addr/2 num=%p, addr=%p\n", &(b->num), &(b->addr));

  
}
