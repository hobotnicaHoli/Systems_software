#include "../inc/relokZapis.hpp"

int relokZapis::prev_id = 0;

relokZapis::relokZapis(){
  offset = 0;
  tip = nullptr;
  simbol = nullptr;
  addend = 0;
  next = nullptr;
  id = ++prev_id;
}

relokZapis::~relokZapis(){
  
}

void relokZapis::postaviOffset(int o){
  offset = o;
}

void relokZapis::postaviTip(char* t){
  tip = t;
}

void relokZapis::postaviSimbol(char* s){
  simbol = s;
}

void relokZapis::postaviAddend(int a){
  addend = a;
}

int relokZapis::dohvatiOffset(){
  return offset;
}

char* relokZapis::dohvatiTip(){
  return tip;
}

char* relokZapis::dohvatiSimbol(){
  return simbol;
}

int relokZapis::dohvatiAddend(){
  return addend;
}


void relokZapis::postaviSekciju(int s){
  sekcija = s;
}

int relokZapis::dohvatiSekciju(){
  return sekcija;
}

int relokZapis::dohvatiId(){
  return id;
}

