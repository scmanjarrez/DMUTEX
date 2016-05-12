#include <stdio.h>
#include <stdlib.h>

int myIndex = 0;
int n_peers = 3;
int peticionAnterior(const int *reqLClk, const int * hisLclk, const int peerIndex)
{
  int i;
  int less=0;
  int leq=0;
  int gr=0;
  int geq=0;

  for (i = 0; i<n_peers; i++)
    {
      if (reqLClk[i] < hisLclk[i]) {
	less++;
      }
      if (reqLClk[i] > hisLclk[i]) {
	gr++;
      } 
      if (reqLClk[i] <= hisLclk[i] && reqLClk[i] >= hisLclk[i] ) {
	geq++;
	leq++;
      }
    }
  if (less>1 && leq==n_peers) {
    return 0;
  }
  else if (gr>1 && geq==n_peers) {
    return 1;
  }

  else {
    return myIndex<peerIndex? 0:1;
  }
}

int main(int argc, char *argv[])
{
  int lclk[3] = {1,1,4};
  int hisLclk[3] = {3,1,3};
  
  printf("el valor devuelto por peticion anterior es %d\n", peticionAnterior(hisLclk, lclk, 2));
  return 0;
}

