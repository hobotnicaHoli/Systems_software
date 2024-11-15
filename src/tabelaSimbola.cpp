#include "../inc/tabelaSimbola.hpp"
#include <stdio.h>
#include <cstring>

tabelaSimbola::tabelaSimbola(){
  head = tail = nullptr;
}

tabelaSimbola::~tabelaSimbola(){
  
}

void tabelaSimbola::ubaciSimbol(Simbol* s){
  if(head == nullptr){
    head = tail = s;
  }
  else{
    tail->next = s;
    tail = s;
  }
}

void tabelaSimbola::ispisiTabeluSimbola(){
  if(head == nullptr){
    printf("Tabela simbola je prazna\n");
    return;
  }
  
  printf("%-10s %-20s %-15s %-15s %-7s %-7s %-7s\n", "Simbol br", "Naziv", "Vrednost", "SectionNum", "Size", "Global", "Extern");

  for(Simbol* s = head; s != nullptr; s = s->next){
    printf("%-10d ", s->id);
    printf("%-20s ", s->naziv);
    printf("%-15x ", (unsigned int)s->vrednost);
    printf("%-15d ", s->sectionNum);
    printf("%-7d ", s->size);
    printf("%-7d ", s->globalBool);
    printf("%-7d\n", s->externBool);
  }
}

Simbol* tabelaSimbola::pronadjiSimbol(char* n){
  Simbol* start = head;
  for(; start != nullptr; start = start->next){
    if(strcmp(n, start->naziv) == 0){
      return start;
    }
  }
  return nullptr;
}