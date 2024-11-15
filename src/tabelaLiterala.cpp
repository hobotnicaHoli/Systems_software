#include "../inc/tabelaLiterala.hpp"
#include <stdio.h>
#include <string.h>

tabelaLiterala::tabelaLiterala(){
  head = nullptr;
  tail = nullptr;
}

tabelaLiterala::~tabelaLiterala(){
  
}

void tabelaLiterala::ubaciLiteral(Literal* l){
  if(head == nullptr){
    head = tail = l;
  }
  else{
    tail->next = l;
    tail = l;
  }
}

Literal* tabelaLiterala::dohvatiLiteral(){
  if(head == nullptr){
    return nullptr;
  }
  Literal* lit = head;
  return lit;
}

void tabelaLiterala::ispisiTabeluLiterala(){
  Literal* l = head;
  while(l != nullptr){
    printf("Literal id: %d\t simbol: %s\t vrednost: %d\n ", l->dohvatiId(), l->dohvatiSimbol(), l->dohvatiVrednost());
    l = l->next;
  }
}

Literal* tabelaLiterala::pronadjiLiteral(int vrednost){
  Literal* l = head;
  while(l != nullptr){
    if(l->dohvatiVrednost() == vrednost){
      return l;
    }
    l = l->next;
  }
  return nullptr;
}


Literal* tabelaLiterala::pronadjiLiteralPoNazivu(char* naziv){
  Literal* l = head;
  while(l != nullptr){
    if(l->dohvatiDaLiJeSimbol() == true){
    if(strcmp(l->dohvatiSimbol(), naziv) == 0){
      return l;
    }
    }
    l = l->next;
  }
  return nullptr;
}
