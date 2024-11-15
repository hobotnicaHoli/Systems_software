#include "../inc/asembler.hpp"
#include "../inc/instrukcija.hpp"
#include <stdio.h>
#include <cmath>
#include <fstream>

using namespace std;

Asembler::Asembler(){
  ts = new tabelaSimbola();
  tsek = new tabelaSekcija();
  prethodnaSekcija = nullptr;
  locationCounter = 0;
}

Asembler::~Asembler(){
  delete ts;
  delete tsek;
}

void Asembler::prviProlaz(){
  if(head == NULL){
    printf("Nema instrukcija!\n");
    exit(-1);
  }
  int returnCode = 0;
  for(struct Instrukcija* ins = head; ins != NULL; ins = ins->next){
    if(ins->direktiva == true){
      returnCode = prviProlazDirektiva(ins);
      if(returnCode == -1){
        printf("Greska pri prvom prolazu asemblera\n");
        exit(-1);
      }
      else if(returnCode == 5){
        //naisla je .end instrukcija
        //treba sve ostalo obrisati
        struct Instrukcija* sledeca = ins->next;
        while(sledeca != NULL){
          struct Instrukcija* pom = sledeca;
          sledeca = sledeca->next;
          delete pom;
        }
      }
    }
    else if(ins->labela == true){
      returnCode = prviProlazLabela(ins);
      if(returnCode != 0){
        printf("Greska pri prvom prolazu asemblera\n");
        exit(-1);
      }
    }
    else{
      returnCode = prviProlazInstrukcija(ins);
      if(returnCode != 0){
        printf("Greska pri prvom prolazu asemblera\n");
        exit(-1);
      }
    }
  }
  if(obradaLiteralnihFlinkova() < 0){
          printf("Greska pri prvom prolazu asemblera\n");
          exit(-1);
  }
  obradaRelokacionihZapisa();
  Simbol* s = ts->head;
  while(s != nullptr){
    if(s->dohvatiExternBool() == false && s->dohvatiGlobalBool() == true && s->dohvatiDefinisanost() == false){
      printf("GRESKA: simbol %s je deklarisan kao global a nije definisan\n", s->dohvatiNaziv());
      exit(-1);
    }
    s = s->next;
  }

  upisiBazenLitUKod();
  
  return;
  
}

int Asembler::prviProlazDirektiva(struct Instrukcija* ins){
  //printf("Obrada direktive %s\n", ins->naziv);
  if(strcmp(ins->naziv, "global") == 0){
    //sadrzi listu simbola kao operand(samo simbola)
    for(struct operand* pom = ins->prvi; pom != nullptr; pom = pom->next){
      Simbol* s = ts->pronadjiSimbol(pom->simbol);
      if(s != nullptr){
        //simbol vec postoji u tabeli simbola
        Sekcija* sek = tsek->pronadjiSekciju(pom->simbol);
        if (sek != nullptr)
        {
          printf("Greska, simbol je vec definisan kao sekcija\n");
          return -1;
        }
        //simbol se vec nalazi u tabeli simbola, nije ga potrebno ubacivati
        s->postaviGlobalBool(true);
      }
      else{
        //simbol ne postoji vec u tabeli simbola
        Sekcija* sek = tsek->pronadjiSekciju(pom->simbol);
        if (sek != nullptr)
        {
          printf("Greska, simbol je vec definisan kao sekcija\n");
          return -1;
        }
        //pravi se nov simbol i postavlja u tabelu simbola
        s = new Simbol();
        s->postaviNaziv(pom->simbol);
        s->postaviGlobalBool(true);
        //posto je global simbol - to je UNDEFINED sekcija
        s->postaviSectionNum(-1);
        s->postaviDefinisanost(false);
        ts->ubaciSimbol(s);
      }
    }
     
  }
  else if(strcmp(ins->naziv, "extern") == 0){
    //isto sadrzi samo listu simbola
    for(struct operand* pom = ins->prvi; pom != nullptr; pom = pom->next){
      Simbol* s = ts->pronadjiSimbol(pom->simbol);
      
      if(s != nullptr){
        //simbol ne postoji vec u tabeli simbola
        if(s->definisan == true){
        printf("GRESKA: simbol ne moze biti definisan u sekciji i proglasen kao extern u istom fajlu\n");
        return -1;
        }
        Sekcija* sek = tsek->pronadjiSekciju(pom->simbol);
        if (sek != nullptr)
        {
          printf("Greska, simbol je vec definisan kao sekcija\n");
          return -1;
        }
        //simbol se vec nalazi u tabeli simbola, nije ga potrebno ubacivati
        s->postaviExternBool(true);
        s->postaviSize(0);
        s->postaviVrednost(0);
        s->postaviGlobalBool(true);
        s->postaviSectionNum(-1);
      }
      else{
        Sekcija* sek = tsek->pronadjiSekciju(pom->simbol);
        if (sek != nullptr)
        {
          printf("Greska, simbol je vec definisan kao sekcija\n");
          return -1;
        }
        s = new Simbol();
        s->postaviNaziv(pom->simbol);
        s->postaviExternBool(true);
        s->postaviGlobalBool(true);
        s->postaviSize(0);
        s->postaviVrednost(0);
        s->postaviSectionNum(-1);
        //simbol se definise u labeli
        s->postaviDefinisanost(false);
        ts->ubaciSimbol(s);
      }
    }
  }
  else if(strcmp(ins->naziv, "section") == 0){
    //sadrzi ime sekcije kao operand(simbol)
    //treba proveriti da li postoji taj naziv sekcije u tabeli simbola - to ne sme
    Simbol* simbol = ts->pronadjiSimbol(ins->prvi->simbol);
    if(simbol != nullptr){
      printf("Greska, sekcija vec postoji u tabeli simbola\n");
      return -1;
    }
    if(prethodnaSekcija != nullptr){
      prethodnaSekcija->postaviVelicinu(locationCounter);
    }
    Sekcija* s = tsek->pronadjiSekciju(ins->prvi->simbol);
    if(s != nullptr){
      //sekcija vec postoji u tabeli sekcija
      printf("Greska, sekcija vec postoji u tabeli sekcija\n");
      return -1;
    }
    else{
      s = new Sekcija();
      s->postaviNaziv(ins->prvi->simbol);
      if(prethodnaSekcija == nullptr){
        s->postaviPocetak(0);
      }
      else{
        int broj = prethodnaSekcija->dohvatiPocetak();
        s->postaviPocetak(locationCounter+broj);
      }
      s->postaviVelicinu(0);
      locationCounter = 0;
      tsek->ubaciSekciju(s);
      
    }
    postaviPrethodnuSekciju(s);
    Simbol* sim = new Simbol();
    sim->postaviNaziv(ins->prvi->simbol);
    sim->postaviSectionNum(s->dohvatiSectionNum());
    sim->postaviSize(0);
    sim->postaviVrednost(0);
    sim->postaviDefinisanost(true);
    sim->postaviSectionBool(true);
    ts->ubaciSimbol(sim);
  }
  else if(strcmp(ins->naziv, "word") == 0){
    //sadrzi listu simbola ili literala
    if(prethodnaSekcija == nullptr){
      printf("Greska, nema trenutne sekcije, direktiva word se mora naci unutar sekcije\n");
      return -1;
    }
    for(struct operand* pom = ins->prvi; pom != nullptr; pom = pom->next){
      if(pom->simbol != nullptr){
        //simbol 
        Simbol* s = ts->pronadjiSimbol(pom->simbol);
        if(s != nullptr){
          //simbol se vec nalazi u tabeli simbola, nije ga potrebno ubacivati
          if(s->dohvatiDefinisanost() == false){
            s->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
            unsigned char c1 = 0x00 & 0xFF;
            unsigned char c2 = (0x00 >> 8) & 0xFF;
            unsigned char c3 = (0x00 >> 16) & 0xFF;
            unsigned char c4 = (0x00 >> 24) & 0xFF;

            prethodnaSekcija->dodajUKod(c1);
            prethodnaSekcija->dodajUKod(c2);
            prethodnaSekcija->dodajUKod(c3);
            prethodnaSekcija->dodajUKod(c4);
          }
          else{
            if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
              //ako je simbol definisan u drugoj sekciji, 0 se upisuje u tab simbola
              unsigned char c1 = 0x00 & 0xFF;
              unsigned char c2 = (0x00 >> 8) & 0xFF;
              unsigned char c3 = (0x00 >> 16) & 0xFF;
              unsigned char c4 = (0x00 >> 24) & 0xFF;

              prethodnaSekcija->dodajUKod(c1);
              prethodnaSekcija->dodajUKod(c2);
              prethodnaSekcija->dodajUKod(c3);
              prethodnaSekcija->dodajUKod(c4);

            }
            else{
              //ako je simbol definisan, moze da se generise masinski kod instrukcije, ako nije mora da se obelezi FLINK
              unsigned char c1 = s->dohvatiVrednost() & 0xFF;
              unsigned char c2 = (s->dohvatiVrednost() >> 8) & 0xFF;
              unsigned char c3 = (s->dohvatiVrednost() >> 16) & 0xFF;
              unsigned char c4 = (s->dohvatiVrednost() >> 24) & 0xFF;

              prethodnaSekcija->dodajUKod(c4);
              prethodnaSekcija->dodajUKod(c3);
              prethodnaSekcija->dodajUKod(c2);
              prethodnaSekcija->dodajUKod(c1);
            }
          }
        }
        else{
          s = new Simbol();
          s->postaviNaziv(pom->simbol);
          s->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
          s->postaviDefinisanost(false);
          ts->ubaciSimbol(s);
          
          unsigned char c1 = 0x00 & 0xFF;
          unsigned char c2 = (0x00 >> 8) & 0xFF;
          unsigned char c3 = (0x00 >> 16) & 0xFF;
          unsigned char c4 = (0x00 >> 24) & 0xFF;

          prethodnaSekcija->dodajUKod(c1);
          prethodnaSekcija->dodajUKod(c2);
          prethodnaSekcija->dodajUKod(c3);
          prethodnaSekcija->dodajUKod(c4);
        }
        //mora da se napravi relokacioni zapis za simbol
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("32");
        rel->postaviTip(tip);
        rel->postaviSimbol(pom->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
        
      }
      else{
        //literal
        //ne smesta se u tabelu literala
        unsigned char c1 = pom->literal & 0xFF;
        unsigned char c2 = (pom->literal >> 8) & 0xFF;
        unsigned char c3 = (pom->literal >> 16) & 0xFF;
        unsigned char c4 = (pom->literal >> 24) & 0xFF;

        prethodnaSekcija->dodajUKod(c4);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c1);
      }
      locationCounter += 4;

    }

  }
  else if(strcmp(ins->naziv, "skip") == 0){
    //sadrzi literal kao operand
    int size = ins->prvi->literal;
    unsigned char c = 0;
    for(int i = 0; i < size; i++){
      prethodnaSekcija->dodajUKod(c);
    }
    locationCounter += ins->prvi->literal;
    
  }
  else if(strcmp(ins->naziv, "end") == 0){
    //nema operand
    
    if(prethodnaSekcija == nullptr){
      printf("Greska, nema prethodne sekcije\n");
      return -1;
    }
    else{
      prethodnaSekcija->postaviVelicinu(locationCounter);
      //gotov prvi prolaz
    }
    return 5;
    //treba prestati sa obradom instrukcija
  }
  else{
    printf("Greska, pogresan naziv direktive\n");
    return -1;
  }
  return 0;
}

int Asembler::prviProlazLabela(struct Instrukcija* ins){
  //printf("Obrada labele %s\n", ins->naziv);
  Simbol* s = ts->pronadjiSimbol(ins->naziv);
  if(prethodnaSekcija == nullptr){
    printf("Greska, nema prethodne sekcije\n");
    return -1;
  }
  if(s != nullptr){
    if(s->dohvatiExternBool() == true){
      printf("Greska, simbol je vec definisan kao extern\n");
      return -1;
    }
    if(s->dohvatiDefinisanost() == true){
      printf("Greska, simbol je vec definisan\n");
      return -1;
    }
    else{
      s->postaviVrednost(locationCounter);
      s->postaviDefinisanost(true);
      s->postaviSectionNum(prethodnaSekcija->dohvatiSectionNum());
      //printf("Postavljen section num: %d\n", s->dohvatiSectionNum());
      if(s->headFlink != nullptr){
        flinkInstrukcija* pom = s->headFlink;
        while(pom != nullptr){
          if(strcmp(pom->instrukcija->naziv, "word") == 0){
          int loccount = pom->lc;

          if(pom->sectionNumber != prethodnaSekcija->dohvatiSectionNum()){
            
            //prepravka se nalazi u drugoj sekciji, ne moze se prepraviti flink, generise se relokacioni zapis
          }
          else{
            //printf("Usao u flink\n");
            //ako je word u pitanju a u istoj sekciji su
          unsigned char c1 = s->dohvatiVrednost() & 0xFF;
          unsigned char c2 = (s->dohvatiVrednost() >> 8) & 0xFF;
          unsigned char c3 = (s->dohvatiVrednost() >> 16) & 0xFF;
          unsigned char c4 = (s->dohvatiVrednost() >> 24) & 0xFF;

          Sekcija* sek = tsek->pronadjiSekNaOsnovuSekNum(pom->sectionNumber);

          sek->masinskiKod[loccount] = c4;
          sek->masinskiKod[loccount + 1] = c3;
          sek->masinskiKod[loccount + 2] = c2;
          sek->masinskiKod[loccount + 3] = c1;

          }
          }

          pom = pom->next;
        }
      }
    }
    
  }
  else{
    s = new Simbol();
    s->postaviNaziv(ins->naziv);
    s->postaviVrednost(locationCounter);
    s->postaviDefinisanost(true);
    //printf("Postavljen section num: %d\n", prethodnaSekcija->dohvatiSectionNum());
    s->postaviSectionNum(prethodnaSekcija->dohvatiSectionNum());
    s->postaviSize(0);
    ts->ubaciSimbol(s);
  }
  return 0;
}

int Asembler::prviProlazInstrukcija(struct Instrukcija* ins){
  //printf("Obrada instrukcije %s\n", ins->naziv);
  if(prethodnaSekcija == nullptr){
    printf("Greska, nema prethodne sekcije\n");
    return -1;
  }
  if(strcmp(ins->naziv, "add") == 0){
    unsigned char c1 = 0x50;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra << 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  } 
  else if(strcmp(ins->naziv, "sub") == 0){
    unsigned char c1 = 0x51;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra<< 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "mul") == 0){
    unsigned char c1 = 0x52;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra << 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "div") == 0){
    unsigned char c1 = 0x53;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra << 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "not") == 0){
    unsigned char c1 = 0x60;
    unsigned char c2 = (unsigned char)(((ins->prvi->brojregistra << 4)&0xF0)|(ins->prvi->brojregistra&0x0F));
    unsigned char c3 = 0x00;
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "and") == 0){
    unsigned char c1 = 0x61;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra<< 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "or") == 0){
    unsigned char c1 = 0x62;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra<< 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "xor") == 0){
    unsigned char c1 = 0x63;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra << 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "shl") == 0){
    unsigned char c1 = 0x70;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra << 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "shr") == 0){
    unsigned char c1 = 0x71;
    unsigned char c2 = (unsigned char)(((ins->drugi->brojregistra<< 4)&0xF0)|(ins->drugi->brojregistra&0x0F));
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra<< 4)&0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "int") == 0){
    unsigned char c1 = 0x10;
    unsigned char c2 = 0x00;
    unsigned char c3 = 0x00;
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);  
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "halt") == 0){
    unsigned char c1 = 0x00;
    unsigned char c2 = 0x00;
    unsigned char c3 = 0x00;
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "push") == 0){
    unsigned char c1 = 0x81;
    unsigned char c2 = 0xE0;
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4)|0x0F);
    unsigned char c4 = 0xFC;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "pop") == 0){
    unsigned char c1 = 0x93;
    unsigned char c2 = (unsigned char)(((ins->prvi->brojregistra << 4)&0xF0)|(0x0E & 0x0F));
    unsigned char c3 = 0x00;
    unsigned char c4 = 0x04;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "xchg") == 0){
    unsigned char c1 = 0x40;
    unsigned char c2 = (unsigned char)(ins->drugi->brojregistra & 0x0F);
    unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4) & 0xF0);
    unsigned char c4 = 0x00;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "ret") == 0){
    unsigned char c1 = 0x93;
    unsigned char c2 = 0xFE;
    unsigned char c3 = 0x00;
    unsigned char c4 = 0x04;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "jmp") == 0){
    if(ins->prvi->brojregistra != -1 || ins->prvi->adr != Adresiranje::MEMDIR){
      printf("Greska, operand za skok ne moze biti registar i adresiranje mora biti memorijsko direktno\n");
      return -1;
    }
    if(ins->prvi->simbol != nullptr){
      //SIMBOL JE U PITANJU
      Simbol* s = ts->pronadjiSimbol(ins->prvi->simbol);

      if(s == nullptr){
        //simbol ni ne postoji u tabeli simbola
        s = new Simbol();
        s->postaviNaziv(ins->prvi->simbol);
        s->postaviDefinisanost(false);
        s->postaviSectionNum(-1);
        ts->ubaciSimbol(s);

        Literal* lit = new Literal();
        lit->postaviDaLiJeSimbol(true);
        lit->postaviSimbol(ins->prvi->simbol);
        prethodnaSekcija->dodajUTabeluLit(lit);
        lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }else{
          //simbol postoji 
          Literal* lit = prethodnaSekcija->pronadjiLiteralPoNazivu(ins->prvi->simbol);
          if(lit == nullptr){
            lit = new Literal();
            lit->postaviDaLiJeSimbol(true);
            lit->postaviSimbol(ins->prvi->simbol);
            prethodnaSekcija->dodajUTabeluLit(lit);
          }
          lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }
      if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
        //ako simbol postoji i nije u istoj sekciji definisan ili ne postoji i tek smo ga napravili i ne znamo jos gde ce biti definisan(u kojoj sekciji) generisemo relok zapis
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("PC32");
        rel->postaviTip(tip);
        rel->postaviSimbol(ins->prvi->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
      }
      unsigned char c1 = 0x30;
      unsigned char c2 = 0xF0;
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      }
    else{
      if(ins->prvi->literal < 4096 && ins->prvi->literal >= -4096){
        unsigned char c1 = 0x30;
        unsigned char c2 = 0x00;
        unsigned char c3 = (unsigned char)(ins->prvi->literal >> 8 & 0x0F);
        unsigned char c4 = (unsigned char)(ins->prvi->literal & 0xFF);
        prethodnaSekcija->dodajUKod(c1);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c4);
      }
      else{
      //LITERAL JE U PITANJU
      Literal* lit = prethodnaSekcija->pronadjiLiteralPoVrednosti(ins->prvi->literal);
      if(lit != nullptr){
        //nista, samo se flink generise svakako jer ne znamo pomeraj literala
      }
      else{
        lit = new Literal();
        lit->postaviDaLiJeSimbol(false);
        lit->postaviVrednost(ins->prvi->literal);
        prethodnaSekcija->dodajUTabeluLit(lit);
      }
      lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      unsigned char c1 = 0x30;
      unsigned char c2 = 0xF0;
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      }
    }
    
  }
  else if(strcmp(ins->naziv, "beq") == 0){
    if(ins->treci->brojregistra != -1 || ins->treci->adr != Adresiranje::MEMDIR){
      printf("Greska, operand za skok ne moze biti registar i adresirannje mora biti memorijsko direktno\n");
      return -1;
    }
    if(ins->treci->simbol != nullptr){
      //SIMBOL JE U PITANJU
      Simbol* s = ts->pronadjiSimbol(ins->treci->simbol);

      if(s == nullptr){
        //simbol ni ne postoji u tabeli simbola
        s = new Simbol();
        s->postaviNaziv(ins->treci->simbol);
        s->postaviDefinisanost(false);
        s->postaviSectionNum(-1);
        ts->ubaciSimbol(s);

        Literal* lit = new Literal();
        lit->postaviDaLiJeSimbol(true);
        lit->postaviSimbol(ins->treci->simbol);
        prethodnaSekcija->dodajUTabeluLit(lit);
        lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }else{
          //simbol postoji 
          Literal* lit = prethodnaSekcija->pronadjiLiteralPoNazivu(ins->treci->simbol);
          if(lit == nullptr){
            lit = new Literal();
            lit->postaviDaLiJeSimbol(true);
            lit->postaviSimbol(ins->treci->simbol);
            prethodnaSekcija->dodajUTabeluLit(lit);
          }
          lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }
      if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
        //ako simbol postoji i nije u istoj sekciji definisan ili ne postoji i tek smo ga napravili i ne znamo jos gde ce biti definisan(u kojoj sekciji) generisemo relok zapis
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("PC32");
        rel->postaviTip(tip);
        rel->postaviSimbol(ins->treci->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
      }
        unsigned char c1 = 0x31;
        unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
        unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|0x00);
        unsigned char c4 = 0x00;
        prethodnaSekcija->dodajUKod(c1);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c4);
      }
      else{
        if(ins->treci->literal < 4096 && ins->prvi->literal >= -4096){
            unsigned char c1 = 0x31;
            unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
            unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|(ins->treci->literal >> 8 & 0x0F));
            unsigned char c4 = (unsigned char)(ins->treci->literal & 0xFF);
            prethodnaSekcija->dodajUKod(c1);
            prethodnaSekcija->dodajUKod(c2);
            prethodnaSekcija->dodajUKod(c3);
            prethodnaSekcija->dodajUKod(c4);
        }
        else{
      //LITERAL JE U PITANJU
      Literal* lit = prethodnaSekcija->pronadjiLiteralPoVrednosti(ins->treci->literal);
      if(lit != nullptr){
        //nista, samo se flink generise svakako jer ne znamo pomeraj literala
      }
      else{
        lit = new Literal();
        lit->postaviDaLiJeSimbol(false);
        lit->postaviVrednost(ins->treci->literal);
        prethodnaSekcija->dodajUTabeluLit(lit);
      }
      lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
        unsigned char c1 = 0x31;
        unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
        unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|0x00);
        unsigned char c4 = 0x00;
        prethodnaSekcija->dodajUKod(c1);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c4);
        }
    }
    
  }
  else if(strcmp(ins->naziv, "bne") == 0){
    if(ins->treci->brojregistra != -1 || ins->treci->adr != Adresiranje::MEMDIR){
      printf("Greska, operand za skok ne moze biti registar i adresirannje mora biti memorijsko direktno\n");
      return -1;
    }
    if(ins->treci->simbol != nullptr){
      //SIMBOL JE U PITANJU
      Simbol* s = ts->pronadjiSimbol(ins->treci->simbol);

      if(s == nullptr){
        //simbol ni ne postoji u tabeli simbola
        s = new Simbol();
        s->postaviNaziv(ins->treci->simbol);
        s->postaviDefinisanost(false);
        s->postaviSectionNum(-1);
        ts->ubaciSimbol(s);

        Literal* lit = new Literal();
        lit->postaviDaLiJeSimbol(true);
        lit->postaviSimbol(ins->treci->simbol);
        prethodnaSekcija->dodajUTabeluLit(lit);
        lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }else{
          //simbol postoji 
          Literal* lit = prethodnaSekcija->pronadjiLiteralPoNazivu(ins->treci->simbol);
          if(lit == nullptr){
            lit = new Literal();
            lit->postaviDaLiJeSimbol(true);
            lit->postaviSimbol(ins->treci->simbol);
            prethodnaSekcija->dodajUTabeluLit(lit);
          }
          lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }
      if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
        //ako simbol postoji i nije u istoj sekciji definisan ili ne postoji i tek smo ga napravili i ne znamo jos gde ce biti definisan(u kojoj sekciji) generisemo relok zapis
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("PC32");
        rel->postaviTip(tip);
        rel->postaviSimbol(ins->treci->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
      }
        unsigned char c1 = 0x32;
        unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
        unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|0x00);
        unsigned char c4 = 0x00;
        prethodnaSekcija->dodajUKod(c1);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c4);
      }
      else{
        if(ins->treci->literal < 4096 && ins->prvi->literal >= -4096){
          unsigned char c1 = 0x32;
          unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
          unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|(ins->treci->literal >> 8 & 0x0F));
          unsigned char c4 = (unsigned char)(ins->treci->literal & 0xFF);
          prethodnaSekcija->dodajUKod(c1);
          prethodnaSekcija->dodajUKod(c2);
          prethodnaSekcija->dodajUKod(c3);
          prethodnaSekcija->dodajUKod(c4);
        }
        else{
      //LITERAL JE U PITANJU
      Literal* lit = prethodnaSekcija->pronadjiLiteralPoVrednosti(ins->treci->literal);
      if(lit != nullptr){
        //nista, samo se flink generise svakako jer ne znamo pomeraj literala
      }
      else{
        lit = new Literal();
        lit->postaviDaLiJeSimbol(false);
        lit->postaviVrednost(ins->treci->literal);
        prethodnaSekcija->dodajUTabeluLit(lit);
      }
      lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      unsigned char c1 = 0x32;
      unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
      unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|0x00);
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
        }
    }
    
  }
  else if(strcmp(ins->naziv, "call") == 0){
    if(ins->prvi->brojregistra != -1 || ins->prvi->adr != Adresiranje::MEMDIR){
      printf("Greska, operand za skok ne moze biti registar i adresirannje mora biti memorijsko direktno\n");
      return -1;
    }
    if(ins->prvi->simbol != nullptr){
      //SIMBOL JE U PITANJU
      Simbol* s = ts->pronadjiSimbol(ins->prvi->simbol);

      if(s == nullptr){
        //simbol ni ne postoji u tabeli simbola
        s = new Simbol();
        s->postaviNaziv(ins->prvi->simbol);
        s->postaviDefinisanost(false);
        s->postaviSectionNum(-1);
        ts->ubaciSimbol(s);

        Literal* lit = new Literal();
        lit->postaviDaLiJeSimbol(true);
        lit->postaviSimbol(ins->prvi->simbol);
        prethodnaSekcija->dodajUTabeluLit(lit);
        lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }else{
          //simbol postoji 
          Literal* lit = prethodnaSekcija->pronadjiLiteralPoNazivu(ins->prvi->simbol);
          if(lit == nullptr){
            lit = new Literal();
            lit->postaviDaLiJeSimbol(true);
            lit->postaviSimbol(ins->prvi->simbol);
            prethodnaSekcija->dodajUTabeluLit(lit);
          }
          lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }
      if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
        //ako simbol postoji i nije u istoj sekciji definisan ili ne postoji i tek smo ga napravili i ne znamo jos gde ce biti definisan(u kojoj sekciji) generisemo relok zapis
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("PC32");
        rel->postaviTip(tip);
        rel->postaviSimbol(ins->prvi->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
      }
        unsigned char c1 = 0x20;
        unsigned char c2 = 0xF0;
        unsigned char c3 = 0x00;
        unsigned char c4 = 0x00;
        prethodnaSekcija->dodajUKod(c1);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c4);
      }
    else{
      if(ins->prvi->literal < 4096 && ins->prvi->literal >= -4096){
        unsigned char c1 = 0x20;
        unsigned char c2 = 0xF0;
        unsigned char c3 = (unsigned char)(ins->prvi->literal >> 8 & 0x0F);
        unsigned char c4 = (unsigned char)(ins->prvi->literal & 0xFF);
        prethodnaSekcija->dodajUKod(c1);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c4);
      }
      else{
      //LITERAL JE U PITANJU
      Literal* lit = prethodnaSekcija->pronadjiLiteralPoVrednosti(ins->prvi->literal);
      if(lit != nullptr){
        //nista, samo se flink generise svakako jer ne znamo pomeraj literala
      }
      else{
        lit = new Literal();
        lit->postaviDaLiJeSimbol(false);
        lit->postaviVrednost(ins->prvi->literal);
        prethodnaSekcija->dodajUTabeluLit(lit);
      }
      lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      unsigned char c1 = 0x20;
      unsigned char c2 = 0xF0;
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      }
    }
    //koja god da je situacija, stavlja se flink jer se ne zna pomeraj literala u tabeli literala dok se ne zavrsi ceo prolaz i sazna se duzina sekcije
    
  }
  else if(strcmp(ins->naziv, "bgt") == 0){
    if(ins->treci->brojregistra != -1 || ins->treci->adr != Adresiranje::MEMDIR){
      printf("Greska, operand za skok ne moze biti registar i adresirannje mora biti memorijsko direktno\n");
      return -1;
    }
    if(ins->treci->simbol != nullptr){
      //SIMBOL JE U PITANJU
      Simbol* s = ts->pronadjiSimbol(ins->treci->simbol);

      if(s == nullptr){
        //simbol ni ne postoji u tabeli simbola
        s = new Simbol();
        s->postaviNaziv(ins->treci->simbol);
        s->postaviDefinisanost(false);
        s->postaviSectionNum(-1);
        ts->ubaciSimbol(s);

        Literal* lit = new Literal();
        lit->postaviDaLiJeSimbol(true);
        lit->postaviSimbol(ins->treci->simbol);
        prethodnaSekcija->dodajUTabeluLit(lit);
        lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }else{
          //simbol postoji 
          Literal* lit = prethodnaSekcija->pronadjiLiteralPoNazivu(ins->treci->simbol);
          if(lit == nullptr){
            lit = new Literal();
            lit->postaviDaLiJeSimbol(true);
            lit->postaviSimbol(ins->treci->simbol);
            prethodnaSekcija->dodajUTabeluLit(lit);
          }
          lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }
      if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
        //ako simbol postoji i nije u istoj sekciji definisan ili ne postoji i tek smo ga napravili i ne znamo jos gde ce biti definisan(u kojoj sekciji) generisemo relok zapis
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("PC32");
        rel->postaviTip(tip);
        rel->postaviSimbol(ins->treci->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
      }
      unsigned char c1 = 0x33;
      unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
      unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|0x00);
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      }
      else{
        if(ins->treci->literal < 4096 && ins->prvi->literal >= -4096){
          unsigned char c1 = 0x33;
          unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
          unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|(ins->treci->literal >> 8 & 0x0F));
          unsigned char c4 = (unsigned char)(ins->treci->literal & 0xFF);
          prethodnaSekcija->dodajUKod(c1);
          prethodnaSekcija->dodajUKod(c2);
          prethodnaSekcija->dodajUKod(c3);
          prethodnaSekcija->dodajUKod(c4);
        }
        else{
      //LITERAL JE U PITANJU
      Literal* lit = prethodnaSekcija->pronadjiLiteralPoVrednosti(ins->treci->literal);
      if(lit != nullptr){
        //nista, samo se flink generise svakako jer ne znamo pomeraj literala
      }
      else{
        lit = new Literal();
        lit->postaviDaLiJeSimbol(false);
        lit->postaviVrednost(ins->treci->literal);
        prethodnaSekcija->dodajUTabeluLit(lit);
      }
      lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      unsigned char c1 = 0x33;
      unsigned char c2 = (unsigned char)((0xF0 )|(ins->prvi->brojregistra&0x0F));
      unsigned char c3 = (unsigned char)((ins->drugi->brojregistra<<4 & 0xF0)|0x00);
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
        }
    }
    
  }
  else if(strcmp(ins->naziv, "iret") == 0){
   
    unsigned char c1 = 0x93;
    unsigned char c2 = 0xFE;
    unsigned char c3 = 0x00;
    unsigned char c4 = 0x08;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
    locationCounter += 4;
    c1 = 0x96;
    c2 = 0x0E;
    c3 = 0x00;
    c4 = 0x04;
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
    
  }
  else if(strcmp(ins->naziv, "ld") == 0){
    if(ins->prvi->adr == Adresiranje::REGDIR){
      unsigned char c1 = 0x91;
      unsigned char c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|(ins->prvi->brojregistra & 0x0F));
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
    }
    else if(ins->prvi->adr == Adresiranje::REGIND){
      unsigned char c1 = 0x92;
      unsigned char c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|(ins->prvi->brojregistra & 0x0F));
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
    }
    else if(ins->prvi->adr == Adresiranje::REGINDPOM){
      if(ins->prvi->simbol != nullptr){
        //simbol je u pitanju
        //u trenutku asembliranja prava vrednost nijednog simbola nije poznata, pa ovaj slucaj ne moze da se desi
      }
      else{
        //literal je u pitanju
        if(ins->prvi->literal > 4095 || ins->prvi->literal < -4096){
          printf("Greska, literal je veci od 4095\n");
          return -1;
        }
        unsigned char c1 = 0x92;
        unsigned char c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|(ins->prvi->brojregistra & 0x0F));
        unsigned char c3 = (unsigned char)((ins->prvi->literal >> 8) & 0x0F);
        unsigned char c4 = (unsigned char)(ins->prvi->literal & 0xFF);
        prethodnaSekcija->dodajUKod(c1);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c4);
      }
    }
    else if(ins->prvi->adr == Adresiranje::IMMED){
    if(ins->prvi->simbol != nullptr){
      //SIMBOL JE U PITANJU
      Simbol* s = ts->pronadjiSimbol(ins->prvi->simbol);

      if(s == nullptr){
        //simbol ni ne postoji u tabeli simbola
        s = new Simbol();
        s->postaviNaziv(ins->prvi->simbol);
        s->postaviDefinisanost(false);
        s->postaviSectionNum(-1);
        ts->ubaciSimbol(s);

        Literal* lit = new Literal();
        lit->postaviDaLiJeSimbol(true);
        lit->postaviSimbol(ins->prvi->simbol);
        prethodnaSekcija->dodajUTabeluLit(lit);
        lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }else{
          //simbol postoji 
          Literal* lit = prethodnaSekcija->pronadjiLiteralPoNazivu(ins->prvi->simbol);
          if(lit == nullptr){
            lit = new Literal();
            lit->postaviDaLiJeSimbol(true);
            lit->postaviSimbol(ins->prvi->simbol);
            prethodnaSekcija->dodajUTabeluLit(lit);
          }
          lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }
      if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
        //ako simbol postoji i nije u istoj sekciji definisan ili ne postoji i tek smo ga napravili i ne znamo jos gde ce biti definisan(u kojoj sekciji) generisemo relok zapis
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("PC32");
        rel->postaviTip(tip);
        rel->postaviSimbol(ins->prvi->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
      }
      unsigned char c1 = 0x92;
      unsigned char c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|0x0F);
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      }
      else{
        if(ins->prvi->literal < 4096 && ins->prvi->literal >= -4096){
          //printf("Broj registra: %d\n", ins->drugi->brojregistra);
          unsigned char c1 = 0x91;
          unsigned char c2 = (unsigned char)(ins->drugi->brojregistra << 4 & 0xF0);
          unsigned char c3 = (unsigned char)(ins->prvi->literal >> 8 & 0x0F);
          unsigned char c4 = (unsigned char)(ins->prvi->literal & 0xFF);
          prethodnaSekcija->dodajUKod(c1);
          prethodnaSekcija->dodajUKod(c2);
          prethodnaSekcija->dodajUKod(c3);
          prethodnaSekcija->dodajUKod(c4);
        }
        else{
      //LITERAL JE U PITANJU
      Literal* lit = prethodnaSekcija->pronadjiLiteralPoVrednosti(ins->prvi->literal);
      if(lit != nullptr){
        //nista, samo se flink generise svakako jer ne znamo pomeraj literala
      }
      else{
        lit = new Literal();
        lit->postaviDaLiJeSimbol(false);
        lit->postaviVrednost(ins->prvi->literal);
        prethodnaSekcija->dodajUTabeluLit(lit);
      }
      lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      unsigned char c1 = 0x92;
      unsigned char c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|0x0F);
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
        }
    }
    
    }
    else if(ins->prvi->adr == Adresiranje::MEMDIR){
      if(ins->prvi->simbol != nullptr){
      //SIMBOL JE U PITANJU
      Simbol* s = ts->pronadjiSimbol(ins->prvi->simbol);

      if(s == nullptr){
        //simbol ni ne postoji u tabeli simbola
        s = new Simbol();
        s->postaviNaziv(ins->prvi->simbol);
        s->postaviDefinisanost(false);
        s->postaviSectionNum(-1);
        ts->ubaciSimbol(s);

        Literal* lit = new Literal();
        lit->postaviDaLiJeSimbol(true);
        lit->postaviSimbol(ins->prvi->simbol);
        prethodnaSekcija->dodajUTabeluLit(lit);
        lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }else{
          //simbol postoji 
          Literal* lit = prethodnaSekcija->pronadjiLiteralPoNazivu(ins->prvi->simbol);
          if(lit == nullptr){
            lit = new Literal();
            lit->postaviDaLiJeSimbol(true);
            lit->postaviSimbol(ins->prvi->simbol);
            prethodnaSekcija->dodajUTabeluLit(lit);
          }
          lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }
      if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
        //ako simbol postoji i nije u istoj sekciji definisan ili ne postoji i tek smo ga napravili i ne znamo jos gde ce biti definisan(u kojoj sekciji) generisemo relok zapis
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("PC32");
        rel->postaviTip(tip);
        rel->postaviSimbol(ins->prvi->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
      }
      unsigned char c1 = 0x92;
      unsigned char c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|0x0F);
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      c1 = 0x92;
      c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|(ins->drugi->brojregistra & 0x0F));
      c3 = 0x00;
      c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      }
      else{
        if(ins->prvi->literal < 4096 && ins->prvi->literal >= -4096){
          unsigned char c1 = 0x92;
          unsigned char c2 = (unsigned char)(ins->drugi->brojregistra << 4 & 0xF0);
          unsigned char c3 = (unsigned char)(ins->prvi->literal >> 8 & 0x0F);
          unsigned char c4 = (unsigned char)(ins->prvi->literal & 0xFF);
          prethodnaSekcija->dodajUKod(c1);
          prethodnaSekcija->dodajUKod(c2);
          prethodnaSekcija->dodajUKod(c3);
          prethodnaSekcija->dodajUKod(c4);
        }
        else{
      //LITERAL JE U PITANJU
      Literal* lit = prethodnaSekcija->pronadjiLiteralPoVrednosti(ins->prvi->literal);
      if(lit != nullptr){
        //nista, samo se flink generise svakako jer ne znamo pomeraj literala
      }
      else{
        lit = new Literal();
        lit->postaviDaLiJeSimbol(false);
        lit->postaviVrednost(ins->prvi->literal);
        prethodnaSekcija->dodajUTabeluLit(lit);
      }
      lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      unsigned char c1 = 0x92;
      unsigned char c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|0x0F);
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      c1 = 0x92;
      c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|(ins->drugi->brojregistra & 0x0F));
      c3 = 0x00;
      c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
        }
    }
    
    //kod memdir + ld lc mora dodatno da se uveca za 4 jer zauzima dve masinske instrukcije
    locationCounter += 4;
    }
  }
  else if(strcmp(ins->naziv, "st") == 0){
    if(ins->drugi->adr == Adresiranje::IMMED){
      //ne sme, GRESKA
      printf("Greska, store ne prihvata IMMED adresiranje!\n");
      return -1;
    }
    else if(ins->drugi->adr == Adresiranje::MEMDIR){
      if(ins->drugi->simbol != nullptr){
      //SIMBOL JE U PITANJU
      Simbol* s = ts->pronadjiSimbol(ins->drugi->simbol);

      if(s == nullptr){
        //simbol ni ne postoji u tabeli simbola
        s = new Simbol();
        s->postaviNaziv(ins->drugi->simbol);
        s->postaviDefinisanost(false);
        s->postaviSectionNum(-1);
        ts->ubaciSimbol(s);

        Literal* lit = new Literal();
        lit->postaviDaLiJeSimbol(true);
        lit->postaviSimbol(ins->drugi->simbol);
        prethodnaSekcija->dodajUTabeluLit(lit);
        lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }else{
          //simbol postoji 
          Literal* lit = prethodnaSekcija->pronadjiLiteralPoNazivu(ins->drugi->simbol);
          if(lit == nullptr){
            lit = new Literal();
            lit->postaviDaLiJeSimbol(true);
            lit->postaviSimbol(ins->drugi->simbol);
            prethodnaSekcija->dodajUTabeluLit(lit);
          }
          lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      }
      if(s->dohvatiSectionNum() != prethodnaSekcija->dohvatiSectionNum()){
        //ako simbol postoji i nije u istoj sekciji definisan ili ne postoji i tek smo ga napravili i ne znamo jos gde ce biti definisan(u kojoj sekciji) generisemo relok zapis
        relokZapis* rel = new relokZapis();
        rel->postaviOffset(locationCounter);
        char* tip = (char*)("PC32");
        rel->postaviTip(tip);
        rel->postaviSimbol(ins->drugi->simbol);
        rel->postaviAddend(0);
        rel->postaviSekciju(prethodnaSekcija->dohvatiSectionNum());
        prethodnaSekcija->dohvatiTabeluRelZap()->ubaciRelokZapis(rel);
      }
      unsigned char c1 = 0x82;
      unsigned char c2 = 0xF0;
      unsigned char c3 = (unsigned char)(ins->prvi->brojregistra << 4 & 0xF0);
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
      }
      else{
        if(ins->drugi->literal < 4096 && ins->prvi->literal >= -4096){
          unsigned char c1 = 0x80;
          unsigned char c2 = 0x00;
          unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4 & 0xF0)|(ins->drugi->literal >> 8 & 0x0F));
          unsigned char c4 = (unsigned char)(ins->drugi->literal & 0xFF);
          prethodnaSekcija->dodajUKod(c1);
          prethodnaSekcija->dodajUKod(c2);
          prethodnaSekcija->dodajUKod(c3);
          prethodnaSekcija->dodajUKod(c4);
        }
        else{
      //LITERAL JE U PITANJU
      Literal* lit = prethodnaSekcija->pronadjiLiteralPoVrednosti(ins->drugi->literal);
      if(lit != nullptr){
        //nista, samo se flink generise svakako jer ne znamo pomeraj literala
      }
      else{
        lit = new Literal();
        lit->postaviDaLiJeSimbol(false);
        lit->postaviVrednost(ins->drugi->literal);
        prethodnaSekcija->dodajUTabeluLit(lit);
      }
      lit->ubaciFlinkInstrukciju(ins, locationCounter, prethodnaSekcija->dohvatiSectionNum());
      unsigned char c1 = 0x82;
      unsigned char c2 = 0xF0;
      unsigned char c3 = (unsigned char)(ins->prvi->brojregistra << 4 & 0xF0);
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
        }
    }
    
    }
    else if(ins->drugi->adr == Adresiranje::REGDIR){
      unsigned char c1 = 0x90;
      unsigned char c2 = (unsigned char)((ins->drugi->brojregistra << 4 & 0xF0)|(ins->prvi->brojregistra & 0x0F));
      unsigned char c3 = 0x00;
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
    }
    else if(ins->drugi->adr == Adresiranje::REGIND){
      unsigned char c1 = 0x80;
      unsigned char c2 = (unsigned char)(ins->drugi->brojregistra << 4 & 0xF0);
      unsigned char c3 = (unsigned char)(ins->prvi->brojregistra << 4 & 0xF0);
      unsigned char c4 = 0x00;
      prethodnaSekcija->dodajUKod(c1);
      prethodnaSekcija->dodajUKod(c2);
      prethodnaSekcija->dodajUKod(c3);
      prethodnaSekcija->dodajUKod(c4);
    }
    else if(ins->drugi->adr == Adresiranje::REGINDPOM){
      if(ins->drugi->simbol != nullptr){
        //simbol je u pitanju
        //u trenutku asembliranja prava vrednost nijednog simbola nije poznata, pa ovaj slucaj ne moze da se desi
      }
      else{
        //literal je u pitanju
        if(ins->drugi->literal > 4095 || ins->drugi->literal < -4096){
          printf("Greska, literal je veci od 4095\n");
          return -1;
        }
        unsigned char c1 = 0x80;
        unsigned char c2 = (unsigned char)(ins->drugi->brojregistra << 4 & 0xF0);
        unsigned char c3 = (unsigned char)((ins->prvi->brojregistra << 4 & 0xF0) | (ins->drugi->literal >> 8 & 0x0F));
        unsigned char c4 = (unsigned char)(ins->drugi->literal & 0xFF);
        prethodnaSekcija->dodajUKod(c1);
        prethodnaSekcija->dodajUKod(c2);
        prethodnaSekcija->dodajUKod(c3);
        prethodnaSekcija->dodajUKod(c4);
      }
    }

  }
  else if(strcmp(ins->naziv, "csrrd")== 0){
    unsigned char c1 = 0x90;
    unsigned char c3 = 0x00;
    unsigned char c4 = 0x00;
    unsigned char c2;
    if(strcmp(ins->prvi->sreg, "status") == 0){
      c2 = (unsigned char)((ins->drugi->brojregistra<< 4 & 0xF0)|0x00);
    }
    else if(strcmp(ins->prvi->sreg, "handler") == 0){
      c2 = (unsigned char)((ins->drugi->brojregistra<< 4 & 0xF0)|0x01);
    }
    else if(strcmp(ins->prvi->sreg, "cause") == 0){
      c2 = (unsigned char)((ins->drugi->brojregistra<< 4 & 0xF0)|0x02);
    }
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  else if(strcmp(ins->naziv, "csrwr") == 0){
    unsigned char c1 = 0x94;
    unsigned char c3 = 0x00;
    unsigned char c4 = 0x00;
    unsigned char c2;
    if(strcmp(ins->drugi->sreg, "status") == 0){
      c2 = (unsigned char)((ins->prvi->brojregistra)|(0x00<<4 & 0xF0));
    }
    else if(strcmp(ins->drugi->sreg, "handler") == 0){
      c2 = (unsigned char)((ins->prvi->brojregistra)|(0x01<<4 & 0xF0));
    }
    else if(strcmp(ins->drugi->sreg, "cause") == 0){
      c2 = (unsigned char)((ins->prvi->brojregistra)|(0x02<<4 & 0xF0));
    }
    prethodnaSekcija->dodajUKod(c1);
    prethodnaSekcija->dodajUKod(c2);
    prethodnaSekcija->dodajUKod(c3);
    prethodnaSekcija->dodajUKod(c4);
  }
  
  else{
    
  }
  locationCounter += 4;

  return 0;
  

}

Sekcija* Asembler::dohvatiPrethodnuSekciju(){
  return prethodnaSekcija;
}
void Asembler::postaviPrethodnuSekciju(Sekcija* s){
  prethodnaSekcija = s;
}

int Asembler::obradaLiteralnihFlinkova(){
  Sekcija* pocetna = tsek->vratiPrvuSekciju();
  while(pocetna != nullptr){
    int velicinasekcije = pocetna->dohvatiVelicinu();
    if(velicinasekcije >= 4096){
      printf("Greska, sekcija je veca od 4096\n");
      return -1;
    }
    int inkrement = 0;
    //idem redom po sekcijama dohvatam tabelu literala za svaku
    Literal* prvi = pocetna->dohvatiTabeluLit()->dohvatiLiteral();
    while(prvi != nullptr){
      
      //idem redom po tabeli literala i dohvatam redom literale
      flinkLiteral* prviflink = prvi->headFlinkLiteral;
      while(prviflink != nullptr){
        //idem redom po literalima i dohvatam redom flinkove
        int pomeraj = 0;
        
        //nasli smo pomeraj sad treba da popunimo masinski kod

        if(strcmp(prviflink->instrukcija->naziv, "call") == 0){
          if(prvi->dohvatiDaLiJeSimbol() == true){
          //simbol je
            Simbol* s = ts->pronadjiSimbol(prvi->dohvatiSimbol());
              if(s->dohvatiSectionNum() == pocetna->dohvatiSectionNum()){
                //ako se simbol nalazi u toj sekciji,moze se upisati njegova vrednost
                pomeraj = s->dohvatiVrednost() - prviflink->lc - 4;
                unsigned char c1 = 0x20;
                unsigned char c2 = 0xF0;
                unsigned char c3 = (unsigned char)(pomeraj >> 8 & 0x0F);
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
              else{
                //simbol nije u istoj sekciji
                pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
                unsigned char c1 = 0x21;
                unsigned char c2 = 0xF0;
                unsigned char c3 = (unsigned char)(pomeraj >> 8 & 0x0F);
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
            }
          else{
            //literal je
            pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
            unsigned char c1 = 0x21;
            unsigned char c2 = 0xF0;
            unsigned char c3 = (unsigned char)(pomeraj >> 8 & 0x0F);
            unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
            pocetna->masinskiKod[prviflink->lc] = c1;
            pocetna->masinskiKod[prviflink->lc + 1] = c2;
            pocetna->masinskiKod[prviflink->lc + 2] = c3;
            pocetna->masinskiKod[prviflink->lc + 3] = c4;
          }
          
        }
        else if(strcmp(prviflink->instrukcija->naziv, "jmp") == 0){
          if(prvi->dohvatiDaLiJeSimbol() == true){
          //simbol je
            Simbol* s = ts->pronadjiSimbol(prvi->dohvatiSimbol());
              if(s->dohvatiSectionNum() == pocetna->dohvatiSectionNum()){
                //ako se simbol nalazi u toj sekciji,moze se upisati njegova vrednost
                pomeraj = s->dohvatiVrednost() - prviflink->lc - 4;
                unsigned char c1 = 0x30;
                unsigned char c2 = 0xF0;
                unsigned char c3 = (unsigned char)(pomeraj >> 8 & 0x0F);
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
              else{
                //simbol nije u istoj sekciji
                pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
                unsigned char c1 = 0x80;
                unsigned char c2 = 0xF0;
                unsigned char c3 = (unsigned char)(pomeraj >> 8 & 0x0F);
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
            }
          else{
            //literal je
            pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
            unsigned char c1 = 0x80;
            unsigned char c2 = 0xF0;
            unsigned char c3 = (unsigned char)(pomeraj >> 8 & 0x0F);
            unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
            pocetna->masinskiKod[prviflink->lc] = c1;
            pocetna->masinskiKod[prviflink->lc + 1] = c2;
            pocetna->masinskiKod[prviflink->lc + 2] = c3;
            pocetna->masinskiKod[prviflink->lc + 3] = c4;
          }
          
        }
        else if(strcmp(prviflink->instrukcija->naziv, "beq") == 0){
          if(prvi->dohvatiDaLiJeSimbol() == true){
          //simbol je
            Simbol* s = ts->pronadjiSimbol(prvi->dohvatiSimbol());
              if(s->dohvatiSectionNum() == pocetna->dohvatiSectionNum()){
                //ako se simbol nalazi u toj sekciji,moze se upisati njegova vrednost
                pomeraj = s->dohvatiVrednost() - prviflink->lc - 4;
                unsigned char c1 = 0x31;
                unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
                unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
              else{
                //simbol nije u istoj sekciji
                pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
                unsigned char c1 = 0x39;
                unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
                unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
            }
          else{
            //literal je
            pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
            unsigned char c1 = 0x39;
            unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
            unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
            unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
            pocetna->masinskiKod[prviflink->lc] = c1;
            pocetna->masinskiKod[prviflink->lc + 1] = c2;
            pocetna->masinskiKod[prviflink->lc + 2] = c3;
            pocetna->masinskiKod[prviflink->lc + 3] = c4;
          }
          
        }
        else if(strcmp(prviflink->instrukcija->naziv, "bne") == 0){
          
          if(prvi->dohvatiDaLiJeSimbol() == true){
          //simbol je
            Simbol* s = ts->pronadjiSimbol(prvi->dohvatiSimbol());
              if(s->dohvatiSectionNum() == pocetna->dohvatiSectionNum()){
                //ako se simbol nalazi u toj sekciji,moze se upisati njegova vrednost
                pomeraj = s->dohvatiVrednost() - prviflink->lc - 4;
                unsigned char c1 = 0x32;
                unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
                unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
              else{
                //simbol nije u istoj sekciji
                pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
                unsigned char c1 = 0x3A;
                unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
                unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
            }
          else{
            //literal je
            pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
            unsigned char c1 = 0x3A;
            unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
            unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
            unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
            pocetna->masinskiKod[prviflink->lc] = c1;
            pocetna->masinskiKod[prviflink->lc + 1] = c2;
            pocetna->masinskiKod[prviflink->lc + 2] = c3;
            pocetna->masinskiKod[prviflink->lc + 3] = c4;
          }
        }
        else if(strcmp(prviflink->instrukcija->naziv, "bgt") == 0){
                    if(prvi->dohvatiDaLiJeSimbol() == true){
          //simbol je
            Simbol* s = ts->pronadjiSimbol(prvi->dohvatiSimbol());
              if(s->dohvatiSectionNum() == pocetna->dohvatiSectionNum()){
                //ako se simbol nalazi u toj sekciji,moze se upisati njegova vrednost
                pomeraj = s->dohvatiVrednost() - prviflink->lc - 4;
                unsigned char c1 = 0x33;
                unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
                unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
              else{
                //simbol nije u istoj sekciji
                pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
                unsigned char c1 = 0x3B;
                unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
                unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;
              }
            }
          else{
            //literal je
            pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
            unsigned char c1 = 0x3B;
            unsigned char c2 = (unsigned char)((0xF0 )|(prviflink->instrukcija->prvi->brojregistra&0x0F));
            unsigned char c3 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra<<4 & 0xF0)|(pomeraj >> 8 & 0x0F));
            unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
            pocetna->masinskiKod[prviflink->lc] = c1;
            pocetna->masinskiKod[prviflink->lc + 1] = c2;
            pocetna->masinskiKod[prviflink->lc + 2] = c3;
            pocetna->masinskiKod[prviflink->lc + 3] = c4;
          }
        }
        else if(strcmp(prviflink->instrukcija->naziv, "ld") == 0){
          if(prviflink->instrukcija->prvi->adr == Adresiranje::IMMED){
            //nebitno da li je simbol ili literal, svakako se na D smesta pomeraj do tog simbola/literala u tabeli literala
                pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
                unsigned char c1 = 0x92;
                unsigned char c2 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra << 4 & 0xF0)|0x0F);
                unsigned char c3 = (unsigned char)(pomeraj >> 8 & 0x0F);
                unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
                pocetna->masinskiKod[prviflink->lc] = c1;
                pocetna->masinskiKod[prviflink->lc + 1] = c2;
                pocetna->masinskiKod[prviflink->lc + 2] = c3;
                pocetna->masinskiKod[prviflink->lc + 3] = c4;

            }

          else if(prviflink->instrukcija->prvi->adr = Adresiranje::MEMDIR){
          
            pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
            unsigned char c1 = 0x92;
            unsigned char c2 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra << 4 & 0xF0)|0x0F);
            unsigned char c3 = (unsigned char)(pomeraj << 8 & 0x0F);
            unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
            pocetna->masinskiKod[prviflink->lc] = c1;
            pocetna->masinskiKod[prviflink->lc + 1] = c2;
            pocetna->masinskiKod[prviflink->lc + 2] = c3;
            pocetna->masinskiKod[prviflink->lc + 3] = c4;
            c1 = 0x92;
            c2 = (unsigned char)((prviflink->instrukcija->drugi->brojregistra << 4 & 0xF0)|(prviflink->instrukcija->drugi->brojregistra & 0x0F));
            c3 = 0x00;
            c4 = 0x00;
            pocetna->masinskiKod[prviflink->lc + 4] = c1;
            pocetna->masinskiKod[prviflink->lc + 5] = c2;
            pocetna->masinskiKod[prviflink->lc + 6] = c3;
            pocetna->masinskiKod[prviflink->lc + 7] = c4;
          
          }
        }
        else if(strcmp(prviflink->instrukcija->naziv, "st") == 0){
          //za MEMDIR slucaj
            pomeraj = velicinasekcije + inkrement - prviflink->lc - 4;
            unsigned char c1 = 0x82;
            unsigned char c2 = 0xF0;
            unsigned char c3 = (unsigned char)((prviflink->instrukcija->prvi->brojregistra << 4 & 0xF0)|(pomeraj >> 8 & 0x0F));
            unsigned char c4 = (unsigned char)(pomeraj & 0xFF);
            pocetna->masinskiKod[prviflink->lc] = c1;
            pocetna->masinskiKod[prviflink->lc + 1] = c2;
            pocetna->masinskiKod[prviflink->lc + 2] = c3;
            pocetna->masinskiKod[prviflink->lc + 3] = c4;
        }
        prviflink = prviflink->next;
      }
      prvi = prvi->next;
      inkrement += 4;
    }
    pocetna = pocetna->next;
  }
  return 0;
}

void Asembler::obradaRelokacionihZapisa(){
  Sekcija* pocetna = tsek->vratiPrvuSekciju();

  //pocetna je prva sekcija
  while(pocetna != nullptr){
    tabelaRelokZapisa* trz = pocetna->dohvatiTabeluRelZap();
    relokZapis* prvi = trz->dohvatiRelokZapis();
    relokZapis* trenutni = prvi;
    //prvi je relokacioni zapis u prvoj sekciji
    while(prvi != nullptr){
      Simbol* s = ts->pronadjiSimbol(prvi->dohvatiSimbol());
      if(s->definisan == false && (s->dohvatiExternBool() == false || s->dohvatiGlobalBool() == false)){
        //simbol se negde random koristi a nije ni definisan ni extern
        printf("Greska, simbol %s nije definisan\n", s->dohvatiNaziv());
        exit(-1);
      }
      if(s->definisan == false && s->dohvatiGlobalBool() == true && s->dohvatiExternBool() == false){
        
        //simbol se deklarise kao global a nigde u fajlu nije definisan
        printf("Greska, simbol %s nije definisan\n", s->dohvatiNaziv());
        exit(-1);
      }
      else if(s->definisan == false && (s->dohvatiExternBool() == true && s->dohvatiGlobalBool() == true)){
        //simbol nije definisan ali je extern - to je ok
        prvi->postaviAddend(-1);
        prvi = prvi->next;
      }
      else{
        Sekcija* sekSimbola = tsek->pronadjiSekNaOsnovuSekNum(s->dohvatiSectionNum());
      if(s->dohvatiSectionNum() == prvi->dohvatiSekciju() && strcmp(prvi->dohvatiTip(), "PC32") == 0){
        //ako je simbol kasnije definisan u toj sekciji, mozemo da izbrisemo relok zapis
        //trenutni je prethodni
        relokZapis* pom = prvi;
        if(trenutni == prvi){
          //prvog treba da izvezem
          trenutni = prvi = prvi->next;
          if(prvi == nullptr){
            trz->head = nullptr;
          }
          trz->head = prvi;
          delete pom;
        }
        else{
          //nije prvi u listi
          trenutni->next = prvi->next;
          prvi = prvi->next;
          if(prvi == nullptr){
            trz->head = nullptr;
          }
          delete pom;
        }
      }
      else{
        //ako simbol nije u istoj sekciji, relok zapis ostaje i mozda moramo da mu prepravimo simbol(u zavisnosti od toga da li je globalan ili lokalan) i addend(isto u zavisnosti od lokalnosti i tipa)
        if(strcmp(prvi->dohvatiTip(), "32") == 0){
          //tip je 32
          if(s->dohvatiGlobalBool() == true){
            //simbol je globalan
            prvi->postaviAddend(0);
          }
          else{
            //simbol je lokalan
            prvi->postaviAddend(s->dohvatiVrednost());
            prvi->postaviSimbol(sekSimbola->dohvatiNaziv());
          }
        }
        else{
          //tip je PC32
          if(s->dohvatiGlobalBool() == true){
            //simbol je globalan
            prvi->postaviAddend(-4);
          }
          else{
            //simbol je lokalan
            prvi->postaviAddend(s->dohvatiVrednost() - 4);
            prvi->postaviSimbol(sekSimbola->dohvatiNaziv());
          }
        }
        trenutni = prvi; //trenutni je prethodni
        prvi = prvi->next;
      }
    }}
    pocetna = pocetna->next;
  }
}

void Asembler::upisiBazenLitUKod(){
  //treba ici redom po svim literalima iz bazena i upisivati za njih po 4 nule u kod
  for(Sekcija* s = tsek->head; s != nullptr; s = s->next){
    tabelaLiterala* tablit = s->dohvatiTabeluLit();
    for(Literal* l = tablit->dohvatiLiteral(); l != nullptr; l = l->next){
      if(l->dohvatiDaLiJeSimbol() == true){
        //simbol je
        unsigned char c1 = 0x00;
        s->dodajUKod(c1);
        s->dodajUKod(c1);
        s->dodajUKod(c1);
        s->dodajUKod(c1);
      }
      else{
        //literal je
        unsigned char c1 = (unsigned char)(l->dohvatiVrednost() >> 24 & 0xFF);
        unsigned char c2 = (unsigned char)((l->dohvatiVrednost() >> 16) & 0xFF);
        unsigned char c3 = (unsigned char)((l->dohvatiVrednost() >> 8) & 0xFF);
        unsigned char c4 = (unsigned char)((l->dohvatiVrednost()) & 0xFF);
        s->dodajUKod(c1);
        s->dodajUKod(c2);
        s->dodajUKod(c3);
        s->dodajUKod(c4);

      }
      int trenutnavel = s->dohvatiVelicinu();
      s->postaviVelicinu(trenutnavel+4);
    }
  }
}

void Asembler::upisiKodUIzlazniFajl(char* nazivFajla){
  ofstream fajl(nazivFajla, ios::binary);
  
  int brSImbola = ts->tail->dohvatiIdSimbola();
  fajl << "SIMBOLI" << " " << brSImbola << "\n";
  Simbol* prvi = ts->head;
  while(prvi != nullptr){
    fajl << prvi->dohvatiIdSimbola() << " " << prvi->dohvatiNaziv() << " " << prvi->dohvatiVrednost() << " " << prvi->dohvatiSectionNum() << " " << prvi->dohvatiGlobalBool() << " " << prvi->dohvatiExternBool() << " " << prvi->dohvatiDefinisanost() << "\n";
    prvi = prvi->next;
  }

  
  int brSekcija = tsek->tail->dohvatiSectionNum();
  fajl << "SEKCIJE" << " " << brSekcija << "\n";
  Sekcija* prva = tsek->vratiPrvuSekciju();
  while(prva != nullptr){
    fajl << prva->dohvatiSectionNum() << " " << prva->dohvatiPocetak() << " "<< prva->dohvatiNaziv() << " " << prva->dohvatiVelicinu() << "\n";
    prva = prva->next;
  }

  
  prva = tsek->vratiPrvuSekciju();
  while(prva != nullptr){
    tabelaRelokZapisa* trz = prva->dohvatiTabeluRelZap();
    relokZapis* pom = trz->dohvatiRelokZapis();
    int brojac = 0;
    while(pom != nullptr){
      brojac++;
      pom = pom->next;
    }
    fajl << "RELOKACIONI ZAPISI" << " " << brojac << "\n";
    relokZapis* prvi = trz->dohvatiRelokZapis();
    while(prvi != nullptr){
      fajl << prvi->dohvatiOffset() << " " << prvi->dohvatiTip() << " " << prvi->dohvatiSimbol() << " " << prvi->dohvatiSekciju() << " " << prvi->dohvatiAddend() << "\n";
      prvi = prvi->next;
    }
    prva = prva->next;
  }

  
  prva = tsek->vratiPrvuSekciju();
  while(prva != nullptr){
    fajl << "MASINSKI KOD" << " " << prva->masinskiKod.size() << "\n";
    for(int i = 0; i < prva->masinskiKod.size(); i++){
      char c1[5];
      sprintf(c1,"%02X",prva->masinskiKod[i]);
      fajl<<string(c1)<<" ";
    }
    fajl << "\n";
    prva = prva->next;
  }
  fajl.close();
}