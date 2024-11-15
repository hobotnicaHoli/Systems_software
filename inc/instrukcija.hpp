#ifndef INSTRUKCIJA_HPP
#define INSTRUKCIJA_HPP

#include <stdlib.h>
#include <string.h>


enum Adresiranje{
  //Tipovi adresiranja
  REGDIR,
  REGIND,
  IMMED,
  MEMDIR,
  REGINDPOM
};

struct operand{
  //potencijalna polja operanda
  int brojregistra;
  char* simbol;
  int literal;
  Adresiranje adr;
  struct operand* next;
  char* sreg;
};

struct Instrukcija{
  //potencijalna polja jedne instrukcije
  char* naziv;
  struct operand* prvi;
  struct operand* drugi;
  struct operand* treci;

  bool direktiva;
  bool labela;

  struct Instrukcija* next;

  
};

extern struct Instrukcija* head;
extern struct Instrukcija* tail;

struct operand* napraviOperand(int regBr, char* s, Adresiranje a, char* sreg);

struct operand* napraviOperandKojiSadrziLiteral(int regBr, int l, Adresiranje a);

void ubaciInstrukciju(char* n, struct operand* p, struct operand* d, struct operand* t);

void ubaciLabelu(char* n);

void ubaciDirektivuBezListe(char* n, struct operand* op);

void ubaciDirektivuSaListom(char* n, struct operand* op);

struct Instrukcija* uzmiInstrukciju();

#endif