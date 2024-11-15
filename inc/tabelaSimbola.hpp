#ifndef TABELASIMBOLA_HPP
#define TABELASIMBOLA_HPP
#include "../inc/simbol.hpp"

class tabelaSimbola{

private:
  

public:
  Simbol* head;
  Simbol* tail;
  void ubaciSimbol(Simbol* s);
  tabelaSimbola();
  ~tabelaSimbola();

  void ispisiTabeluSimbola();

  Simbol* pronadjiSimbol(char* n);

};

#endif