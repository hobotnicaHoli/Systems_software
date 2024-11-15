#ifndef TABELALITERALA_HPP
#define TABELALITERALA_HPP
#include "../inc/literal.hpp"


class tabelaLiterala{
  private:
    Literal* head;
    Literal* tail;
  public:

    tabelaLiterala();
    ~tabelaLiterala();
    void ubaciLiteral(Literal* l);
    Literal* dohvatiLiteral();
    void ispisiTabeluLiterala();
    Literal* pronadjiLiteral(int vrednost);
    Literal* pronadjiLiteralPoNazivu(char* naziv);

};


#endif