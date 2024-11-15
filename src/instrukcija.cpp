#include "../inc/instrukcija.hpp"
#include <stdio.h>

struct Instrukcija* head = NULL;
struct Instrukcija* tail = NULL;


void ubaciInstrukciju(char* n, struct operand* p, struct operand* d, struct operand* t){
  struct Instrukcija* newI = (Instrukcija*)malloc(sizeof(struct Instrukcija));
  newI->naziv = n;
  newI->prvi = p;
  newI->drugi = d;
  newI->treci = t;
  newI->direktiva = false;
  newI->labela = false;
  newI->next = NULL;
  
  if(head == NULL){
    head = tail = newI;
  }
  else{
    tail->next = newI;
    tail = newI;
  }

}

void ubaciLabelu(char* n){
  struct Instrukcija* newI = (Instrukcija*)malloc(sizeof(struct Instrukcija));
  newI->naziv = n;
  newI->direktiva = false;
  newI->labela = true;
  newI->next = NULL;

  if(head == NULL){
    head = tail = newI;
  }
  else{
    tail->next = newI;
    tail = newI;
  }
}

struct Instrukcija* uzmiInstrukciju(){
  if(head == NULL){
    return NULL;
  }
  struct Instrukcija* i = head;
  if(head->next == NULL){
    head = tail = NULL;
  }
  else{
    head = head->next;
  }

  return i;
}

struct operand* napraviOperand(int regBr, char* s, Adresiranje a, char* sreg){
  struct operand* o = (operand*)malloc(sizeof(struct operand));
  o->brojregistra = regBr;
  o->simbol = s;
  o->adr = a;
  o->sreg = sreg;
  o->next = NULL;
  return o;
}

struct operand* napraviOperandKojiSadrziLiteral(int regBr, int l, Adresiranje a){
  struct operand* o = (operand*)malloc(sizeof(struct operand));
  o->brojregistra = regBr;
  o->literal = l;
  o->adr = a;
  o->next = NULL;
  return o;
}

void ubaciDirektivuBezListe(char* n, struct operand* op){
  struct Instrukcija* newI = (Instrukcija*)malloc(sizeof(struct Instrukcija));
  newI->naziv = n;
  newI->prvi = op;
  newI->next = NULL;
  newI->direktiva = true;
  newI->labela = false;

  if(head == NULL){
    head = tail = newI;
  }
  else{
    tail->next = newI;
    tail = newI;
  }
}

void ubaciDirektivuSaListom(char* n, struct operand* op){
  struct Instrukcija* newI = (Instrukcija*)malloc(sizeof(struct Instrukcija));
  newI->naziv = n;
  newI->prvi = op;
  newI->next = NULL;
  newI->direktiva = true;
  newI->labela = false;

  if(head == NULL){
    head = tail = newI;
  }
  else{
    tail->next = newI;
    tail = newI;
  }
}