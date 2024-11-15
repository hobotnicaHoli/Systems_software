#ifndef TABELASEKCIJA_HPP
#define TABELASEKCIJA_HPP

#include "../inc/sekcija.hpp"

class tabelaSekcija{
  private:
    
  public:
  Sekcija* head;
    Sekcija* tail;
    tabelaSekcija();
    ~tabelaSekcija();
    void ubaciSekciju(Sekcija* s);
    void ispisiTabeluSekcija();
    Sekcija* pronadjiSekciju(char* n);
    Sekcija* pronadjiSekcijuNaOsnovuRednogBroja(int rb);
    Sekcija* pronadjiSekNaOsnovuSekNum(int num);
    void zaSveSekcijeIspisiMasinskiKod();
    void zaSveSekcijeIspisiTabeluLiterala();  
    void zaSveSekcijeIspisiTabeluRelokZapisa();

    Sekcija* vratiPrvuSekciju();
};


#endif