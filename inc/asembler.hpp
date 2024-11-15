#ifndef ASEMBLER_HPP
#define ASEMBLER_HPP
#include <stdio.h>
#include <iostream>
#include "../inc/tabelaSimbola.hpp"
#include "../inc/tabelaSekcija.hpp"


class Asembler{

private:
  int locationCounter;
  
  

public:
  Asembler();

  tabelaSimbola* ts;

  tabelaSekcija* tsek;

  Sekcija* prethodnaSekcija;

  ~Asembler();

  Sekcija* dohvatiPrethodnuSekciju();

  void postaviPrethodnuSekciju(Sekcija* s);

  void prviProlaz();

  int prviProlazDirektiva(struct Instrukcija* ins);

  int prviProlazLabela(struct Instrukcija* ins);

  int prviProlazInstrukcija(struct Instrukcija* ins);

  int obradaLiteralnihFlinkova();

  void obradaRelokacionihZapisa();

  void upisiKodUIzlazniFajl(char* nazivFajla);

  void upisiBazenLitUKod();

};


#endif