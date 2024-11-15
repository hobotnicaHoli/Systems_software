#ifndef TABELARELOKZAPISA_HPP
#define TABELARELOKZAPISA_HPP
#include "../inc/relokZapis.hpp"

class tabelaRelokZapisa{
private:
  
  

public:
relokZapis* head;
relokZapis* tail;
  tabelaRelokZapisa();
  ~tabelaRelokZapisa();
  void ubaciRelokZapis(relokZapis* rz);
  relokZapis* dohvatiRelokZapis();
  void ispisiTabeluRelokZapisa();
  relokZapis* pronadjiRelokZapisPoOfsetu(int offset);
  relokZapis* pronadjiRelokZapisPoSimbolu(char* simbol);

};


#endif