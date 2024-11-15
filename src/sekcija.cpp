#include "../inc/sekcija.hpp"
#include <stdio.h>

Sekcija::Sekcija(){
  velicina = 0;
  SectionNum = ++prev_id;
  tabelaLit = new tabelaLiterala();
  tabelaRelZap = new tabelaRelokZapisa();
}

Sekcija::~Sekcija(){
  
}

int Sekcija::prev_id = 0;

void Sekcija::postaviNaziv(char* n){
  naziv = n;
}

unsigned int Sekcija::dohvatiPocetak(){
  return pocetak;
}

void Sekcija::postaviPocetak(unsigned int p){
  pocetak = p;
}

void Sekcija::postaviVelicinu(int v){
  velicina = v;
}

int Sekcija::dohvatiVelicinu(){
  return velicina;
}

void Sekcija::postaviSectionNum(int num){
  SectionNum = num;

}

int Sekcija::dohvatiSectionNum(){
  return SectionNum;
}

void Sekcija::ispisiMasinskiKod(){
  int brojac = 1;
  printf(": ");
  for(vector<unsigned char>::iterator it = masinskiKod.begin(); it != masinskiKod.end(); it++){
    printf("%02X ", *it);
    if(brojac%4 != 0){
      printf(" ");
    }
    else{
      printf("\n");
      printf(": ");
    }
    brojac++;
  }
  printf("\n");
}


void Sekcija::dodajUKod(unsigned char c){
  masinskiKod.push_back(c);
}

void Sekcija::dodajUTabeluLit(Literal* l){
  tabelaLit->ubaciLiteral(l);
}

Literal* Sekcija::pronadjiLiteralPoVrednosti(int v){
  return tabelaLit->pronadjiLiteral(v);
}

void Sekcija::ispisiTabeluLiterala(){
  tabelaLit->ispisiTabeluLiterala();
}

Literal* Sekcija::pronadjiLiteralPoNazivu(char* naziv){
  return tabelaLit->pronadjiLiteralPoNazivu(naziv);
}

tabelaLiterala* Sekcija::dohvatiTabeluLit(){
  return tabelaLit;
}

tabelaRelokZapisa* Sekcija::dohvatiTabeluRelZap(){
  return tabelaRelZap;
}

char* Sekcija::dohvatiNaziv(){
  return naziv;
}