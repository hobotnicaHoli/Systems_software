#include "../inc/simbol.hpp"
#include <stdio.h>
#include <stdlib.h>

int Simbol::prev_id = 0;

Simbol::Simbol(){
  next = nullptr;
  headFlink = nullptr;
  tailFlink = nullptr;
  definisan = false;
  externBool = false;
  id = ++prev_id;
}

Simbol::~Simbol(){
  struct flinkInstrukcija* tek = headFlink;
  struct flinkInstrukcija* stari;
  while(tek != nullptr){
    stari = tek;
    tek = tek->next;
    free(stari);
  }
}

void Simbol::postaviNaziv(char* n){
  naziv = n;
}

void Simbol::postaviVrednost (int v){
  vrednost = v;
}

void Simbol::postaviSectionNum(int s){
  sectionNum = s;
}

void Simbol::postaviSize(int s){
  size = s;
}

void Simbol::postaviGlobalBool(bool b){
  globalBool = b;
}

void Simbol::postaviExternBool(bool b){
  externBool = b;
}

void Simbol::ubaciFlinkInstrukciju(struct Instrukcija* ins, int lc, int sc){
    struct flinkInstrukcija* novi = (struct flinkInstrukcija*)malloc(sizeof(struct flinkInstrukcija));
    novi->instrukcija = ins;
    novi->lc = lc;
    novi->sectionNumber = sc;
    novi->next = nullptr;
    if(headFlink == nullptr){
        headFlink = tailFlink = novi;
    }
    else{
        tailFlink->next = novi;
        tailFlink = novi;
    }

}

int Simbol::dohvatiVrednost(){
    return vrednost;
}

struct Instrukcija* Simbol::dohvatiFlinkInstrukciju(){
    if(headFlink == nullptr){
        return nullptr;
    }
    struct Instrukcija* i = headFlink->instrukcija;
    if(headFlink->next == nullptr){
        headFlink = tailFlink = nullptr;
    }
    else{
        headFlink = headFlink->next;
    }

    return i;
}

void Simbol::postaviDefinisanost(bool d){
    definisan = d;
}

bool Simbol::dohvatiDefinisanost(){
    return definisan;
}

void Simbol::postaviSectionBool(bool b){
    sectionBool = b;
}

bool Simbol::dohvatiSectionBool(){
    return sectionBool;
}

int Simbol::dohvatiSectionNum(){
    return sectionNum;
}

bool Simbol::dohvatiGlobalBool(){
    return globalBool;
}

bool Simbol::dohvatiExternBool(){
    return externBool;
}

char* Simbol::dohvatiNaziv(){
    return naziv;
}

int Simbol::dohvatiIdSimbola(){
    return id;
}