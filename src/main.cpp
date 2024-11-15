#include "../misc/lexer.hpp"
#include "../misc/parser.hpp"
#include "../inc/instrukcija.hpp"
#include "../inc/asembler.hpp"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "../inc/asembler.hpp"
#include "../inc/tabelaSimbola.hpp"

extern int yyparse(void);

int main(int argc, char** argv){
  if(argc!=4||strcmp(argv[1],"-o")!=0){
    printf("Greska, pogresno pozivanje asemblera!\n");
    return -1;
  }
  FILE* file = fopen(argv[3], "r");
  //printf("Otvaram fajl: %s\n", argv[3]);

  if(!file){
    return -2;
  }

  yyin = file;

  int i = yyparse();
  if(i != 0){
    printf("GRESKA PRI PARSIRANJU!\n");
    return -3;
  }

  //printf("Gotovo parsiranje!\n");

  
  Asembler* as = new Asembler();
  as->prviProlaz();
  

  //printf("Tabela simbola:\n");
  //as->ts->ispisiTabeluSimbola();
  
  //printf("Tabela sekcija: \n");
  //as->tsek->ispisiTabeluSekcija();

  //as->tsek->zaSveSekcijeIspisiTabeluLiterala();

  //as->tsek->zaSveSekcijeIspisiTabeluRelokZapisa();

  //printf("Masinski kod: \n");
  //as->tsek->zaSveSekcijeIspisiMasinskiKod();

  as->upisiKodUIzlazniFajl(argv[2]);
  
  fclose(file);
  delete as;
  return 0;
}