#ifndef LINKER_HPP
#define LINKER_HPP
#include <iostream>
#include <fstream>
#include <string>
#include "tabelaSimbola.hpp"
#include "tabelaSekcija.hpp"
#include "relokZapis.hpp"
#include "sekcija.hpp"
#include <list>
#include <map>

class Linker{
  private: 
  tabelaSimbola* ts;
  tabelaSekcija* tsek;
  vector<string> inputfiles;
  vector<int> fajlIDsekcija;
  string outputfile;
  multimap<uint32_t, string> SectionPlaces;
  vector<string> smesteneSekcije;
public:

  static int prev_id;
  int id;
    Linker(vector<string> i, string o, multimap<uint32_t, string> pl){
      this->inputfiles = i;
      this->outputfile = o;
      this->SectionPlaces = pl;
      ts = new tabelaSimbola();
      tsek = new tabelaSekcija();
    }
    ~Linker();

    tabelaSekcija* dohvatiTabSekcija();

    tabelaSimbola* dohvatiTabSimbola();

    void ucitaj1Fajl(char* nazivFajla);

    Sekcija* pronadjiSekcijuNaOsnovuSecNum(int num);

    void smestiSekcije();

    void proveriSimbole();

    void prepraviKod();

    void prepraviSimbole();

    void prepraviSimboleEksterne();

    void ispisiUHexFajl();

};
#endif