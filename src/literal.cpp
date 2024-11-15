#include "../inc/literal.hpp"
#include <stdio.h>
#include <stdlib.h>

int Literal::prev_id = 0;

Literal::Literal(){
  id = ++prev_id;
  next = nullptr;
  simbol = nullptr;
  daLiJeSimbol = false;
}

Literal::~Literal(){
  struct flinkLiteral* tek = headFlinkLiteral;
  struct flinkLiteral* stari;
  while(tek != nullptr){
    stari = tek;
    tek = tek->next;
    free(stari);
  }
}

void Literal::postaviVrednost(int v){
  vrednost = v;
}
void Literal::postaviId(int i){
  id = i;
}
int Literal::dohvatiId(){
  return id;
}
int Literal::dohvatiVrednost(){
  return vrednost;
}
void Literal::postaviNext(Literal* l){
  next = l;
}
void Literal::postaviPozicijuUTabeli(int p){
  pozicijaUTabeli = p;
}
int Literal::dohvatiPozicijuUTabeli(){
  return pozicijaUTabeli;
}

void Literal::postaviSimbol(char* s){
  simbol = s;
}

void Literal::postaviDaLiJeSimbol(bool b){
  daLiJeSimbol = b;
}

char* Literal::dohvatiSimbol(){
  return simbol;
}

bool Literal::dohvatiDaLiJeSimbol(){
  return daLiJeSimbol;
}


void Literal::ubaciFlinkInstrukciju(struct Instrukcija* ins, int lc, int sn){
    struct flinkLiteral* novi = (struct flinkLiteral*)malloc(sizeof(struct flinkLiteral));
    novi->instrukcija = ins;
    novi->lc = lc;
    novi->sectionNumber = sn;
    novi->next = nullptr;
    if(headFlinkLiteral == nullptr){
        headFlinkLiteral = tailFlinkLiteral = novi;
    }
    else{
        tailFlinkLiteral->next = novi;
        tailFlinkLiteral = novi;
    }

}
