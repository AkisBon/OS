#ifndef MODEL_IMPL.H
#define MODEL_IMPL.H
#endif

#include <stdio.h>	// for printf()
#include <stdlib.h>	// for random()
#include <string.h>
#include "model_impl.h"

int NR_VIRT_PAGES	= 5;
int NR_PHYS_PAGES =	3;
int NR_REFERENCES	= 24;

int main(int argc, char **argv){

    if (argc != 5 && argc != 6){ //check number of arguments
      printf("5 or 6 arguments must be given!\n");
      return -1;
    }

    int max = -1;
    if (argc == 6) { // if max is given
      max = atoi(argv[5]);
    }
    //intialize all argc
    char repl_arg[4];  memcpy(repl_arg, argv[1], 4);
    int framesNum = atoi(argv[2]);
    int q = atoi(argv[3]);
    int k = atoi(argv[4]);

    //repl frames q k
    virtualMemory(framesNum, k, q, max, repl_arg);

}
