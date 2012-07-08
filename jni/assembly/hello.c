#include <stdio.h>


extern void pgBitBlitBilinearS(void * d, void * vptr);

void * d = 	(void *)1122;
void * vptr = 	(void *)2233;


int main(int argc, char * args)
{
  //printf("videopointers: %d:%d \n", d, vptr);
  pgBitBlitBilinearS(d, vptr);
  return 1;
}
