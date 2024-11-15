#include "../inc/tabelaRelokZapisa.hpp"
#include <string.h>
#include <stdio.h>

tabelaRelokZapisa::tabelaRelokZapisa(){
  head = nullptr;
  tail = nullptr;
}

tabelaRelokZapisa::~tabelaRelokZapisa(){
  
}

void tabelaRelokZapisa::ubaciRelokZapis(relokZapis* rz){
  if(head == nullptr){
    head = tail = rz;
  }
  else{
    tail->next = rz;
    tail = rz;
  }
}

relokZapis* tabelaRelokZapisa::dohvatiRelokZapis(){
  if(head == nullptr){
    return nullptr;
  }
  relokZapis* rz = head;

  return rz;
}

void tabelaRelokZapisa::ispisiTabeluRelokZapisa(){
  relokZapis* rz = head;
  printf("%-15s %-15s %-15s %-15s\n", "Offset", "Tip", "Simbol", "Addend");
  while(rz != nullptr){
    printf("%-15d %-15s %-15s %-15d\n", rz->dohvatiOffset(), rz->dohvatiTip(), rz->dohvatiSimbol(), rz->dohvatiAddend());
    rz = rz->next;
  }
}

relokZapis* tabelaRelokZapisa::pronadjiRelokZapisPoOfsetu(int offset){
  relokZapis* rz = head;
  while(rz != nullptr){
    if(rz->dohvatiOffset() == offset){
      return rz;
    }
    rz = rz->next;
  }
  return nullptr;
}

relokZapis* tabelaRelokZapisa::pronadjiRelokZapisPoSimbolu(char* simbol){
  relokZapis* rz = head;
  while(rz != nullptr){
    if(strcmp(rz->dohvatiSimbol(), simbol) == 0){
      return rz;
    }
    rz = rz->next;
  }
  return nullptr;
}

