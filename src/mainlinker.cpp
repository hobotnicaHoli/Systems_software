#include <stdio.h>
#include <fstream>
#include <iostream>

#include <string.h>
#include "../inc/linker.hpp"
#include <map>
#include <list>
using namespace std;

int main(int argc, char** argv){
  //linker ulaz -o izlaz
  printf("Usao u linker\n");
  if(argc < 5 || string(argv[1]).compare("-hex") != 0){
    cerr << "Bad arguments" << endl;
    exit(-1);
  }
  int i;
  multimap<uint32_t, string> places;
  for(i = 2; i < argc; i++){
    string arg = string(argv[i]);
    if(arg.substr(0, 7).compare("-place=") == 0){
      string sectionName;
      int position = arg.find_first_of('@');
      sectionName = arg.substr(7, position - 7);
      string hexPosition = arg.substr(position + 1);
      
      places.insert(make_pair(stol(hexPosition, 0, 16), sectionName));

    }
    else{
      break;
    }
  }
  if(string(argv[i]).compare("-o") != 0){
    cerr << "Bad arguments: -o is missing" << endl;
    exit(-1);
  }
  i++;
  vector<string> inputfiles;
  string outputfile = argv[i++];
  while(i < argc){
    inputfiles.push_back(argv[i]);
    i++;
  }


  Linker* l = new Linker(inputfiles, outputfile, places);

  for(int j = 0; j < inputfiles.size(); j++){
    const int length = inputfiles[j].size(); 
  
    // declaring character array (+1 for null terminator) 
    char* char_array = new char[length + 1]; 
    // copying the contents of the 
    // string to char array 
    strcpy(char_array, inputfiles[j].c_str());
    l->ucitaj1Fajl(char_array);
  }

  l->smestiSekcije();

  l->proveriSimbole();

  l->prepraviSimbole();
  
  l->prepraviSimboleEksterne();

  l->prepraviKod();
  
  printf("--------------------------------------------------------------------------------------------\n");
  printf("\t \t \t TABELA SIMBOLA\n");
  l->dohvatiTabSimbola()->ispisiTabeluSimbola();
  printf("\n");

  printf("--------------------------------------------------------------------------------------------\n");
  printf("\t \t \t TABELA SEKCIJA\n");
  l->dohvatiTabSekcija()->ispisiTabeluSekcija();
  printf("\n");

  printf("--------------------------------------------------------------------------------------------\n");
  printf("\t \t \t RELOKACIONI ZAPISI\n");
  l->dohvatiTabSekcija()->zaSveSekcijeIspisiTabeluRelokZapisa();
  printf("\n");

  printf("--------------------------------------------------------------------------------------------\n");
  printf("\t \t \t MASINSKI KOD\n");
  l->dohvatiTabSekcija()->zaSveSekcijeIspisiMasinskiKod();
  printf("\n");


  l->ispisiUHexFajl();
  
  delete l;
  return 0;
}