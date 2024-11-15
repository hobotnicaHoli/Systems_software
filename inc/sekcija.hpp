#ifndef SEKCIJA_HPP
#define SEKCIJA_HPP
#include "../inc/tabelaLiterala.hpp"
#include "../inc/tabelaRelokZapisa.hpp"
#include <vector>
using namespace std;

class Sekcija{

  friend class tabelaSekcija;
  private:
    char* naziv;
    unsigned int pocetak;
    int velicina;
    int SectionNum;
    tabelaLiterala* tabelaLit;
    tabelaRelokZapisa* tabelaRelZap;
    
    
  public:
  vector<unsigned char> masinskiKod;
  Sekcija* next;
    Sekcija();
    ~Sekcija();
    static int prev_id;
    void postaviNaziv(char* n);
    void postaviPocetak(unsigned int p);
    void postaviVelicinu(int v);
    void postaviSectionNum(int num);
    int dohvatiSectionNum();
    int dohvatiVelicinu();
    unsigned int dohvatiPocetak();

    tabelaLiterala* dohvatiTabeluLit();

    static bool compareByPocetak(const Sekcija& a, const Sekcija& b){
      return a.pocetak < b.pocetak;
    }

    tabelaRelokZapisa* dohvatiTabeluRelZap();

    void dodajUTabeluLit(Literal* l);
    Literal* pronadjiLiteralPoVrednosti(int v);
    Literal* pronadjiLiteralPoNazivu(char* naziv);

    void ispisiMasinskiKod();

    void dodajUKod(unsigned char c);

    char* dohvatiNaziv();

    void ispisiTabeluLiterala();
};


#endif