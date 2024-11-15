#ifndef EMULATOR_HPP
#define EMULATOR_HPP
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <string>
#include <fstream>
using namespace std;


class Emulator{

  private:

    string ulazniFajl;
    
    int creg[3]; // status - 0, handler - 1, cause - 2
    unsigned char* mem;

  public:
    unsigned int reg[15]; // sp - 14, pc - 15
    Emulator(string uf);
    ~Emulator();
    void izvrsavanjeInstrukcija();
    void inicijalizacijaIUcitavanje();
    void ispisiMemoriju();
    void ispisiRegistre();

    void ispisiStek();

    void neispravno();

};


#endif