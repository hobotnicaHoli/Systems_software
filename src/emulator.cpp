#include "../inc/emulator.hpp"
#include <vector>
#include <sstream>

Emulator::Emulator(string uf){
  ulazniFajl = uf;
  mem = nullptr;
}

Emulator::~Emulator(){
  
}

void Emulator::inicijalizacijaIUcitavanje(){

  for(int i = 0; i < 16; i++){
    
    reg[i] = 0;
  }
  reg[15] = 0x40000000;
  for(int i = 0; i < 3; i++){
    creg[i] = 0;
  }
  size_t length=1UL<<32;
  //inicijalizacija i punjenje memorije
  mem=(unsigned char*)mmap(nullptr,length,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE,-1,0);
  if(mem==MAP_FAILED){
    printf("Greska prilikom alokacije memorije\n");
    return;
  }
  ifstream ulaz(ulazniFajl);
  if(ulaz.fail()){
    printf("Greska prilikom otvaranja ulaznog fajla\n");
    return;
  }
  string linija;
  vector<string> bajtovi;
  while(getline(ulaz, linija)){
    bajtovi.clear();
    stringstream stringovi(linija);
    string bajt;
    while(stringovi >> bajt){
      bajtovi.push_back(bajt);
    }
    //.front() cita prvi element u vektoru
    string adr = bajtovi.front();
    //pretvaranje u int adresu iz stringa
    unsigned int memadr = (unsigned int)stol(adr, nullptr, 16);
    //na te adrese se redom ispisuje sadrzaj tj instrukcije
    for(auto i = bajtovi.begin() + 2; i != bajtovi.end(); i++){
      mem[memadr++] = (unsigned char)stol((*i), nullptr, 16);
    }
  }
  ulaz.close();
}

void Emulator::izvrsavanjeInstrukcija(){
  bool end = false;
  while(!end){
    printf("Reg[15]: %x\n", reg[15]);
    unsigned char c4 = mem[reg[15]];
    unsigned char c3 = mem[reg[15] + 1];
    unsigned char c2 = mem[reg[15] + 2];
    unsigned char c1 = mem[reg[15] + 3];
    reg[15] += 4;
    unsigned char opCode = (c1 >> 4) & 0x0F;
    printf("OP CODE: %d, ", opCode);
    
    unsigned char mod = c1 & 0x0F;
    printf("MOD: %d, ", mod);
    unsigned char regA = (c2 >> 4) & 0x0F;
    unsigned char regB = c2 & 0x0F;
    unsigned char regC = (c3 >> 4) & 0x0F;
    int pom;
    printf("RegA: %d, RegB: %d, RegC: %d, ", regA, regB, regC);
    unsigned char d = (c3 >> 3) & 0x01;
    if(d == 1){
      //negativan broj
      //popunjava od 12. do 31. bita sa 1
      pom = 0xFFFFF000 | ((c3 & 0x0F) << 8) | c4;
    }
    else{
      //pozitivan broj
      //popunjava od 12. do 31. bita sa 0
      pom = 0x00000FFF & (((c3 & 0x0F) << 8) | c4);
    }
    printf("Pom: %d\n", pom);
    switch(opCode){
      case 0:{
        //halt
        end = true;
        break;
      }
      case 1:{
        //int
        reg[14] = reg[14] - 4;
        mem[reg[14]] = (unsigned char)(creg[0] & 0xFF);
        mem[reg[14]+1] = (unsigned char)((creg[0]>>8) & 0xFF);
        mem[reg[14]+2] = (unsigned char)((creg[0]>>16) & 0xFF);
        mem[reg[14]+3] = (unsigned char)((creg[0]>>24) & 0xFF);
        reg[14] = reg[14] - 4;
        mem[reg[14]] = (unsigned char)(reg[15] & 0xFF);
        mem[reg[14]+1] = (unsigned char)((reg[15]>>8) & 0xFF);
        mem[reg[14]+2] = (unsigned char)((reg[15]>>16) & 0xFF);
        mem[reg[14]+3] = (unsigned char)((reg[15]>>24) & 0xFF);
        creg[2] = 4;
        creg[0] = creg[0] & (~0x1);
        reg[15] = creg[1];
        break;
      }
      case 2:{
        //call
        switch(mod){
          case 0:{
            //call u istoj sekciji
            reg[14] = reg[14] - 4;
            mem[reg[14]] = (unsigned char)(reg[15] & 0xFF);
            mem[reg[14]+1] = (unsigned char)((reg[15]>>8) & 0xFF);
            mem[reg[14]+2] = (unsigned char)((reg[15]>>16) & 0xFF);
            mem[reg[14]+3] = (unsigned char)((reg[15]>>24) & 0xFF);
            reg[15] = reg[regA] + reg[regB] + pom;
            break;
          }
          case 1:{
            //call preko tabele literala
            reg[14] = reg[14] - 4;
            mem[reg[14]] = (unsigned char)(reg[15] & 0xFF);
            mem[reg[14]+1] = (unsigned char)((reg[15]>>8) & 0xFF);
            mem[reg[14]+2] = (unsigned char)((reg[15]>>16) & 0xFF);
            mem[reg[14]+3] = (unsigned char)((reg[15]>>24) & 0xFF);
            unsigned int lokacija = (unsigned int)(reg[regA] + reg[regB]+pom);
            unsigned char c1 = mem[lokacija];
            unsigned char c2 = mem[lokacija+1];
            unsigned char c3 = mem[lokacija+2];
            unsigned char c4 = mem[lokacija+3];
            //mem[reg[A]+reg[B] + pom] je adresa na kojoj se nalazi instrukcija na koju se skace
            reg[15] = (c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            break;
          }
          default:{
            //neispravno
            neispravno();
            break;
          }
        }
        break;
      }
      case 3:{
        switch(mod){
          case 0:{
            //jmp u istoj sekciji
            reg[15] = reg[regA] + pom;
            break;
          }
          case 1:{
            //beq u istoj sekciji
            if(reg[regB] == reg[regC]){
              reg[15] = reg[regA] + pom;
            }
            break;
          }
          case 2:{
            //bne u istoj sekciji
            if(reg[regB] != reg[regC]){
              reg[15] = reg[regA] + pom;
            }
            break;
          }
          case 3:{
            //bgt u istoj sekciji
            if(reg[regB] > reg[regC]){
              reg[15] = reg[regA] + pom;
            }
            break;
          }
          case 8:{
            //jmp preko tabele literala
            unsigned char c1 = mem[reg[regA] + pom];
            unsigned char c2 = mem[reg[regA] + pom + 1];
            unsigned char c3 = mem[reg[regA] + pom + 2];
            unsigned char c4 = mem[reg[regA] + pom + 3];
            reg[15] = (c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            break;
          }
          case 9:{
            //beq preko tabele literala
            if(reg[regB] == reg[regC]){
              unsigned char c1 = mem[reg[regA] + pom];
              unsigned char c2 = mem[reg[regA] + pom + 1];
              unsigned char c3 = mem[reg[regA] + pom + 2];
              unsigned char c4 = mem[reg[regA] + pom + 3];
              reg[15] = (c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            }
            break;
          }
          case 10:{
            //bne preko tabele literala
            if(reg[regB] != reg[regC]){
              unsigned char c1 = mem[reg[regA] + pom];
              unsigned char c2 = mem[reg[regA] + pom + 1];
              unsigned char c3 = mem[reg[regA] + pom + 2];
              unsigned char c4 = mem[reg[regA] + pom + 3];
              reg[15] = (c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            }
            break;
          }
          case 11:{
            //bgt preko tabele literala
            if(reg[regB] > reg[regC]){
              unsigned char c1 = mem[reg[regA] + pom];
              unsigned char c2 = mem[reg[regA] + pom + 1];
              unsigned char c3 = mem[reg[regA] + pom + 2];
              unsigned char c4 = mem[reg[regA] + pom + 3];
              reg[15] = (c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            }
            break;
          }
          default:{
            //neispravno
            neispravno();
          }
        }
        break;
      }
      case 4:{
        //xchg
        int pomocni = reg[regA];
        reg[regA] = reg[regB];
        reg[regB] = pomocni;
        break;
      }
      case 5:{
        switch(mod){
          case 0:{
            //add
            reg[regA] = reg[regB] + reg[regC];
            break;
          }
          case 1:{
            //sub
            reg[regA] = reg[regB] - reg[regC];
            break;
          }
          case 2:{
            //mul
            reg[regA] = reg[regB] * reg[regC];
            break;
          }
          case 3:{
            //div
            double f=reg[regB]/reg[regC];
            reg[regA]=(int)(f);
            break;
          }
          default:{
            //neispravno
            neispravno();
            break;
          }

        }
        break;
      }
      case 6:{
        switch(mod){
          case 0:{
            //not
            reg[regA] = ~reg[regB];
            break;
          }
          case 1:{
            //and
            reg[regA] = reg[regB] & reg[regC];
            break;
          }
          case 2:{
            //or
            reg[regA] = reg[regB] | reg[regC];
            break;
          }
          case 3:{
            //xor
            reg[regA] = reg[regB] ^ reg[regC];
            break;
          }
          default:{
            //neispravno
            neispravno();
            break;
          }
        }
        break;
      }
      case 7:{
        switch(mod){
          case 0:{
            //shl
            reg[regA] = reg[regB] << reg[regC];
            break;
          }
          case 1:{
            //shr
            reg[regA] = reg[regB] >> reg[regC];
            break;
          }
          default:{
            //neispravno
            neispravno();
            break;
          }
        }
        break;
      }
      case 8:{
        switch(mod){
          case 0:{
            //st
            unsigned int lokacija = (unsigned int)(reg[regA] + reg[regB] + pom);
            mem[lokacija] = (unsigned char)(reg[regC]&0xFF);
            mem[lokacija+1] = (unsigned char)((reg[regC]>>8)&0xFF);
            mem[lokacija+2] = (unsigned char)((reg[regC]>>16)&0xFF);
            mem[lokacija+3] = (unsigned char)((reg[regC]>>24)&0xFF);
            break;

          }
          case 1:{
            //push
            
            reg[regA] = reg[regA] + pom;
            unsigned int lokacija = (unsigned int)(reg[regA]);
            mem[lokacija] = (unsigned char)(reg[regC]&0xFF);
            mem[lokacija+1] = (unsigned char)((reg[regC]>>8)&0xFF);
            mem[lokacija+2] = (unsigned char)((reg[regC]>>16)&0xFF);
            mem[lokacija+3] = (unsigned char)((reg[regC]>>24)&0xFF);
            break;
          }
          case 2:{
            //st
            unsigned int lokacija = (unsigned int)(reg[regA] + reg[regB] + pom);
            unsigned char c1 = mem[lokacija];
            unsigned char c2 = mem[lokacija+1];
            unsigned char c3 = mem[lokacija+2];
            unsigned char c4 = mem[lokacija+3];
            lokacija = (unsigned int)(c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            mem[lokacija] = (unsigned char)(reg[regC]&0xFF);
            mem[lokacija+1] = (unsigned char)((reg[regC]>>8)&0xFF);
            mem[lokacija+2] = (unsigned char)((reg[regC]>>16)&0xFF);
            mem[lokacija+3] = (unsigned char)((reg[regC]>>24)&0xFF);
            break;
          }
          default:{
            //neispravno
            neispravno();
            break;
          }
        }
        break;
      }
      case 9:{
        switch(mod){
          case 0:{
            //csrrd
            reg[regA] = creg[regB];
            break;
          }
          case 1:{
            //ld,st
            reg[regA] = reg[regB] + pom;
            break;
          }
          case 2:{
            //ld
            unsigned int lokacija = (unsigned int)(reg[regB] + reg[regC] + pom);
            unsigned char c1 = mem[lokacija];
            unsigned char c2 = mem[lokacija+1];
            unsigned char c3 = mem[lokacija+2];
            unsigned char c4 = mem[lokacija+3];
            reg[regA] = (c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            break;
          }
          case 3:{
            //pop
            //upise se lokacija u memoriji sa koje se cita podatak tj sa stek pointera
            unsigned int lokacija = (unsigned int)reg[regB];
            //to sa stek pointera(+0,+1,+2,+3) se upisuje u registarA
            reg[regA] = (mem[lokacija+3]<<24) | (mem[lokacija+2]<<16) | (mem[lokacija+1]<<8) | mem[lokacija];
            //stek pointer se povecava za 4
            reg[regB] = reg[regB] + pom;
            break;
          }
          case 4:{
            //csrwr
            creg[regA] = reg[regB];
            break;
          }
          case 6:{
            //iret
            unsigned int lokacija = (unsigned int)(reg[regB] + reg[regC] + pom);
            unsigned char c1 = mem[lokacija];
            unsigned char c2 = mem[lokacija+1];
            unsigned char c3 = mem[lokacija+2];
            unsigned char c4 = mem[lokacija+3];
            creg[regA] = (c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            break;
          }
          case 7:{
            //iret
            unsigned int lokacija = (unsigned int)(reg[regB]);
            unsigned char c1 = mem[lokacija];
            unsigned char c2 = mem[lokacija+1];
            unsigned char c3 = mem[lokacija+2];
            unsigned char c4 = mem[lokacija+3];
            creg[regA] = (c1 & 0xFF) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF) << 16) | ((c4 & 0xFF) << 24);
            reg[regB] = reg[regB] + pom;
            break;
          }
          default:{
            //neispravno
            neispravno();
            break;
          }
        }
        break;

      }
      default:{
        //neispravno
        neispravno();
        break;
      }
    }
    reg[0] = 0;
  } 
}

void Emulator::neispravno(){
  printf("Neispravno\n");
  //generise se int(softverski prekid) sa statusom 1 koji oznacava da je nekorektna instrukcija
  reg[14] = reg[14] - 4;
  mem[reg[14]] = (unsigned char)(creg[0] & 0xFF);
  mem[reg[14]+1] = (unsigned char)((creg[0]>>8) & 0xFF);
  mem[reg[14]+2] = (unsigned char)((creg[0]>>16) & 0xFF);
  mem[reg[14]+3] = (unsigned char)((creg[0]>>24) & 0xFF);
  reg[14] = reg[14] - 4;
  mem[reg[14]] = (unsigned char)(reg[15] & 0xFF);
  mem[reg[14]+1] = (unsigned char)((reg[15]>>8) & 0xFF);
  mem[reg[14]+2] = (unsigned char)((reg[15]>>16) & 0xFF);
  mem[reg[14]+3] = (unsigned char)((reg[15]>>24) & 0xFF);
  creg[2] = 1;
  //creg[1] = 0;
  creg[0] = creg[0] & (~0x1);
  reg[15] = creg[1];
}

void Emulator::ispisiMemoriju(){
  for(int i = 2000; i < 5300; i++){
    printf("%02X ", mem[i]);
    if(i%4 == 3){
      printf("\n");
    }
  }
}

void Emulator::ispisiStek(){
  printf("Stek: \n");
  for(unsigned int i = 0xFFFFFEFE; i >= 0xFFFFFED2; i -=4){
    unsigned char c1 = mem[i];
    unsigned char c2 = mem[i+1];
    unsigned char c3 = mem[i+2];
    unsigned char c4 = mem[i+3];
    printf("%08X : %02X %02X %02X %02X\n",i,  c1, c2, c3, c4);
  }
  printf("\n");
}

void Emulator::ispisiRegistre(){
  printf("--------------------------------------------------\n");
  printf("Emulated processor executed halt instruction\n");
  printf("Emulated processor state:\n");
  int t = 1;
  for(int i = 0; i < 16; i++){
    
    printf("r%d=", i);
    printf("0x%08X", reg[i]);;
    if(t % 4 == 0){
      printf("\n");
      t = 1;
    }
    else{
      printf("\t");
      t++;
    }
  }
}
