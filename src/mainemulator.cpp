#include <stdio.h>
#include <fstream>
#include <iostream>

#include <string.h>
#include "../inc/emulator.hpp"
using namespace std;

int main(int argc, char** argv){
  printf("Usao u emulator\n");
  if(argc != 2){
    cerr << "Bad arguments" << endl;
    exit(-1);
  }
  Emulator* e = new Emulator(argv[1]);
  e->inicijalizacijaIUcitavanje();

  //e->ispisiMemoriju();
  //printf("PC: %d\n", e->reg[15]);

  e->izvrsavanjeInstrukcija();
  e->ispisiRegistre();
  delete e;
  return 0;
}