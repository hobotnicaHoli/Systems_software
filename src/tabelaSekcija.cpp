#include "../inc/tabelaSekcija.hpp"
#include <stdio.h>
#include <iostream>
#include <cstring>


tabelaSekcija::tabelaSekcija(){
  head = nullptr;
  tail = nullptr;
}

tabelaSekcija::~tabelaSekcija(){
  
}

void tabelaSekcija::ubaciSekciju(Sekcija* s){
  if(head == nullptr){
    head = tail = s;
  }
  else{
    tail->next = s;
    tail = s;
  }
  tail->next = nullptr;
}

void tabelaSekcija::ispisiTabeluSekcija(){
  Sekcija* tek = head;
  printf("%-20s %-15s %-15s %-15s\n", "Naziv", "Pocetak", "Velicina", "SectionNum");
  for(; tek != nullptr; tek = tek->next){
    printf("%-20s ", tek->naziv);
    printf("%-15x ", (unsigned int)tek->pocetak);
    printf("%-15d ", tek->velicina);
    printf("%-15d\n", tek->SectionNum);
  }
}

Sekcija* tabelaSekcija::pronadjiSekciju(char* n){
  Sekcija* start = head;
  for(; start != nullptr; start = start->next){
    if(strcmp(n, start->naziv) == 0){
      return start;
    }
  }
  return nullptr;
}

void tabelaSekcija::zaSveSekcijeIspisiMasinskiKod(){
  Sekcija* tek = head;
  for(; tek != nullptr; tek = tek->next){
    printf("Sekcija: %s\n", tek->naziv);
    tek->ispisiMasinskiKod();
  }
}

Sekcija* tabelaSekcija::pronadjiSekcijuNaOsnovuRednogBroja(int rb){
  Sekcija* start = head;
  for(int i = 0; start != nullptr; start = start->next, i++){
    if(i == rb){
      return start;
    }
  }
  return nullptr;
}


void tabelaSekcija::zaSveSekcijeIspisiTabeluLiterala(){
  printf("Tabela literala: \n");
  Sekcija* tek = head;
  for(; tek != nullptr; tek = tek->next){
    printf("Sekcija: %s\n", tek->naziv);
    tek->ispisiTabeluLiterala();
  }
}

Sekcija* tabelaSekcija::vratiPrvuSekciju(){
  return head;
}

void tabelaSekcija::zaSveSekcijeIspisiTabeluRelokZapisa(){
  printf("Tabela relok zapisa: \n");
  Sekcija* tek = head;
  for(; tek != nullptr; tek = tek->next){
    printf("Sekcija: %s\n", tek->naziv);
    tek->dohvatiTabeluRelZap()->ispisiTabeluRelokZapisa();
  }
}

Sekcija* tabelaSekcija::pronadjiSekNaOsnovuSekNum(int num){
  Sekcija* start = head;
  for(; start != nullptr; start = start->next){
    if(start->SectionNum == num){
      return start;
    }
  }
  return nullptr;
}