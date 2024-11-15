#include "../inc/linker.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <istream>
#include <string.h>
#include <iomanip>
#include <algorithm>
using namespace std;

int Linker::prev_id = 0;


Linker::~Linker(){
  delete ts;
  delete tsek;
}

tabelaSekcija* Linker::dohvatiTabSekcija(){
  return tsek;
}

tabelaSimbola* Linker::dohvatiTabSimbola(){
  return ts;
}

void Linker::ucitaj1Fajl(char* nazivFajla){
  ifstream file(nazivFajla);
  if(!file.is_open()){
    cout<<"Greska pri otvaranju fajla!"<<endl;
    return;
  }
  string linija1;
  //prvo idu simboli
  getline(file, linija1);
  
  vector<string> reci1;
  stringstream ss1(linija1);
  string simbol1;
  while(ss1>>simbol1){
    reci1.push_back(simbol1);
  }
  int brojSimbola = stoi(reci1[1]);
  
  for(int i = 0; i < brojSimbola; i++){
    getline(file, linija1);
    reci1.clear();
    stringstream ss2(linija1);
    string simbol2;
    while(ss2>>simbol2){
      reci1.push_back(simbol2);
    }
    Simbol* s = new Simbol();
    s->postaviNaziv(strdup(reci1[1].c_str()));
    s->postaviVrednost((unsigned int)stoi(reci1[2]));
    s->postaviSectionNum(stoi(reci1[3]));
    s->postaviGlobalBool(stoi(reci1[4]) == 1? true : false);
    s->postaviExternBool(stoi(reci1[5]) == 1? true : false);
    s->postaviDefinisanost(stoi(reci1[6]) == 1? true : false);
    s->postaviNazivFajla(nazivFajla);
    s->postaviSize(0);
    ts->ubaciSimbol(s);
  }
  
  getline(file, linija1);
    reci1.clear();
    stringstream ss4(linija1);
    string simbol4;
    while(ss4>>simbol4){
      reci1.push_back(simbol4);
    }
  int brojSekcija = stoi(reci1[1]);
  for(int i = 0; i < brojSekcija; i++){
    getline(file, linija1);
    reci1.clear();
    stringstream ss3(linija1);
    string simbol3;
    while(ss3>>simbol3){
      reci1.push_back(simbol3);
    }
    Sekcija* s = new Sekcija();
    int secNum = stoi(reci1[0]);
    int tren_id = ++prev_id;
    fajlIDsekcija.push_back(tren_id);
    s->postaviSectionNum(tren_id);
    Simbol* sim = ts->head;
    while(sim != nullptr){
      if(sim->dohvatiSectionNum() == secNum && sim->dohvatiNazivFajla() == nazivFajla){
        sim->postaviSectionNum(tren_id);
      }
      sim = sim->next;
    }
    s->postaviPocetak((unsigned int)stoi(reci1[1]));
    s->postaviNaziv(strdup(reci1[2].c_str()));
    s->postaviVelicinu(stoi(reci1[3]));
    tsek->ubaciSekciju(s);
  }
  
for(int i = fajlIDsekcija.at(0); i <= fajlIDsekcija.at(fajlIDsekcija.size() -1); i++){
    Sekcija* s = pronadjiSekcijuNaOsnovuSecNum(i);
    getline(file, linija1);
    reci1.clear();
    stringstream ss5(linija1);
    string simbol5;
    while(ss5>>simbol5){
      reci1.push_back(simbol5);
    }
  int brojRelokZapisa = stoi(reci1[2]);
  for(int i = 0;i<brojRelokZapisa;i++){
    getline(file, linija1);
    reci1.clear();
    stringstream ss6(linija1);
    string simbol6;
    while(ss6>>simbol6){
      reci1.push_back(simbol6);
    }
    relokZapis* rz = new relokZapis();
    rz->postaviOffset(stoi(reci1[0]));
    rz->postaviTip(strdup(reci1[1].c_str()));
    rz->postaviSimbol(strdup(reci1[2].c_str()));
    rz->postaviSekciju(stoi(reci1[3]));
    rz->postaviAddend(stoi(reci1[4]));
    s->dohvatiTabeluRelZap()->ubaciRelokZapis(rz);
  }
  }
  for(int i = fajlIDsekcija.at(0); i <= fajlIDsekcija.at(fajlIDsekcija.size() - 1); i++){
    Sekcija* s = pronadjiSekcijuNaOsnovuSecNum(i);
    getline(file, linija1);
    reci1.clear();
    stringstream ss7(linija1);
    string simbol7;
    while(ss7>>simbol7){
      reci1.push_back(simbol7);
    }
  int brojInstrukcija = stoi(reci1[2]);
  getline(file, linija1);
    reci1.clear();
    stringstream ss8(linija1);
    string simbol8;
    while(ss8>>std::hex>>simbol8){
      reci1.push_back(simbol8);
    }
  for(int i = 0;i<brojInstrukcija;i++){
    
    unsigned char c1=(unsigned char)(stoi(reci1[i],0,16));
    s->masinskiKod.push_back(c1);
  }
  }
  fajlIDsekcija.clear();
  }
  
Sekcija* Linker::pronadjiSekcijuNaOsnovuSecNum(int num){
  Sekcija* s = tsek->head;
  while(s != nullptr){
    if(s->dohvatiSectionNum() == num){
      return s;
    }
    s = s->next;
  }
  
  return nullptr;
}

void Linker::smestiSekcije(){
  uint32_t trenutniPlace = 0;
  //prvo treba ici kroz map<uint32_t, string> SectionPlaces
  //treba nekako sortirati place-ove
  for(auto it = SectionPlaces.begin(); it != SectionPlaces.end(); it++){
    printf("Section: %s, place: %x\n", it->second.c_str(), it->first);
  }
  //vec je sortirano po place-u
  //treba da se vade redom sekcije iz SectionPlaces 
  //treba da se merge-uju sekcije sa istim imenom
  //bolje je da se ne radi merge to je komplikovano zbog relok zapisa
  //bolje samo da ih poredjam jednu iza druge
  for(auto it = SectionPlaces.begin(); it != SectionPlaces.end(); it++){
    const int length = it->second.size();

    // declaring character array (+1 for null terminator) 
    char* char_array = new char[length + 1]; 
    // copying the contents of the 
    // string to char array 
    strcpy(char_array, it->second.c_str());
    Sekcija* s = tsek->pronadjiSekciju(char_array);
    if(s == nullptr){
      printf("Sekcija %s nije pronadjena\n", char_array);
      exit(-1);
    }
    if(trenutniPlace > it->first){
      printf("Greska u rasporedu sekcija\n");
      exit(-1);
    }
    s->postaviPocetak((unsigned int)it->first);
    trenutniPlace = it->first;
    trenutniPlace += s->dohvatiVelicinu();
    smesteneSekcije.push_back(s->dohvatiNaziv());

    //provera da li postoji jos neka sekcija sa tim imenom koju treba postaviti iza nje
    for(Sekcija* sek = tsek->head; sek != nullptr; sek = sek->next){
      if(strcmp(sek->dohvatiNaziv(), s->dohvatiNaziv()) == 0 && sek->dohvatiPocetak() != it->first){
        sek->postaviPocetak((unsigned int)trenutniPlace);
        trenutniPlace += sek->dohvatiVelicinu();
      }
    }
  }
  //sad treba sve ostale sekcije da rasporedim iza redom dalje
  for(Sekcija* sek = tsek->head; sek != nullptr; sek = sek->next){
    //treba da proverim da li se nalazi u SectionPlaces
    //ako se ne nalazi onda treba da je rasporedim iza redom
    bool postoji = false;
    for(auto it = SectionPlaces.begin(); it != SectionPlaces.end(); it++){
      if(strcmp(sek->dohvatiNaziv(), it->second.c_str()) == 0){
        postoji = true;
        break;
      }
    }
    if(postoji == false){
      //nije vec postavljena sekcija nigde
      bool smestena = false;
      for(int i = 0; i < smesteneSekcije.size(); i++){
        if(strcmp(sek->dohvatiNaziv(), smesteneSekcije.at(i).c_str()) == 0){
          smestena = true;
          break;
        }
      }
      if(smestena != true){
        sek->postaviPocetak((unsigned int)trenutniPlace);
        trenutniPlace += sek->dohvatiVelicinu();
        smesteneSekcije.push_back(sek->dohvatiNaziv());

      //opet treba da proverim da li ima neka sekcija sa istim takvim imenom
      for(Sekcija* sek2 = tsek->head; sek2 != nullptr; sek2 = sek2->next){
        if(strcmp(sek2->dohvatiNaziv(), sek->dohvatiNaziv()) == 0 && sek2->dohvatiPocetak() != sek->dohvatiPocetak()){
          sek2->postaviPocetak((unsigned int)trenutniPlace);
          trenutniPlace += sek2->dohvatiVelicinu();
        }
      }
      }
      
    }
  }
  printf("Poslagane sekcije: \n");
  printf("%-15s %-10s %-10s\n", "Naziv", "Duzina", "Pocetak");
  for(Sekcija* sek = tsek->head; sek != nullptr; sek = sek->next){
    
    printf("%-15s %-10d %-10x\n", sek->dohvatiNaziv(), sek->dohvatiVelicinu(), sek->dohvatiPocetak());
  }
}

void Linker::proveriSimbole(){
  //treba proveriti da li postoje dva iskljucivo global simbola sa istim imenom
  //takodje treba proveriti za extern+global simbol da li postoji iskljucivo global simbol sa istim imenom
  for(Simbol* sim = ts->head; sim != nullptr; sim = sim->next){
    if(sim->dohvatiGlobalBool() == true && sim->dohvatiExternBool() == false && sim->dohvatiDefinisanost() == false){
      printf("Greska: Globalni simbol %s nije definisan\n", sim->dohvatiNaziv());
      exit(-1);
    }
    if(sim->dohvatiGlobalBool() == true && sim->dohvatiExternBool() == false){
      for(Simbol* sim2 = ts->head; sim2 != nullptr; sim2 = sim2->next){
        if(sim != sim2 && sim2->dohvatiGlobalBool() == true && sim2->dohvatiExternBool() == false && strcmp(sim->dohvatiNaziv(), sim2->dohvatiNaziv()) == 0){
          printf("Greska: Postoje dva global simbola sa istim imenom, konflikt imena: %s\n", sim->dohvatiNaziv());
          exit(-1);
        }
      }
    }
    if(sim->dohvatiGlobalBool() == true && sim->dohvatiExternBool() == true){
      //to je extern simbol
      //treba videti da li postoji neki "drugi" iskljucivo global simbol sa istim imenom
      bool postoji = false;
      for(Simbol* sim2 = ts->head; sim2 != nullptr; sim2 = sim2->next){
        if(sim != sim2 && sim2->dohvatiGlobalBool() == true && sim2->dohvatiExternBool() == false && strcmp(sim->dohvatiNaziv(), sim2->dohvatiNaziv()) == 0){
          postoji = true;
        }
      }
      if(postoji == false){
        printf("Greska: Postoji extern simbol: %s koji nema odgovarajuci global simbol\n", sim->dohvatiNaziv());
        exit(-1);
      }
    }
  }
      printf("Simboli provereni!\n");
}

void Linker::prepraviSimbole(){
  //ides redom kroz tab simbola
  //ako je simbol ni globalni ni eksterni samo na njegovu vrednost nadodam pocetak sekcije kojoj pripada
  //ako je simbol globalni a nije eksterni isto to uradim
  //ako je simbol eksterni onda nadjem isti takav po imenu koji je samo globalni i njegovu vrednost+pocetak sekcije upisem
  for(Simbol* s = ts->head; s != nullptr; s = s->next){
    if(s->dohvatiGlobalBool() == false && s->dohvatiExternBool() == false){
      int prethodnavrednost = s->dohvatiVrednost();
      Sekcija* sek = pronadjiSekcijuNaOsnovuSecNum(s->dohvatiSectionNum());
      if(sek == nullptr){
        //simbol nije definisan, svakako se baca greska
        s->postaviVrednost(-1);
      }
      else{
        s->postaviVrednost(prethodnavrednost + sek->dohvatiPocetak());

      }
      
    }
    else if(s->dohvatiGlobalBool() == true && s->dohvatiExternBool() == false){
      int prethodnavrednost = s->dohvatiVrednost();
      Sekcija* sek = pronadjiSekcijuNaOsnovuSecNum(s->dohvatiSectionNum());
      if(sek == nullptr){
        //simbol nije definisan, svakako se baca greska
        s->postaviVrednost(-1);
      }
      else{
        s->postaviVrednost(prethodnavrednost + sek->dohvatiPocetak());
      }
    }
    
  }
}

void Linker::prepraviSimboleEksterne(){
  for(Simbol* s = ts->head; s != nullptr; s = s->next){
    if(s->dohvatiGlobalBool() == true && s->dohvatiExternBool() == true){
      for(Simbol* s2 = ts->head; s2 != nullptr; s2 = s2->next){
        if(s2->dohvatiGlobalBool() == true && s2->dohvatiExternBool() == false && s != s2 && strcmp(s->dohvatiNaziv(), s2->dohvatiNaziv()) == 0){
          s->postaviVrednost(s2->dohvatiVrednost());
        }
      }
    }
  }
}

void Linker::prepraviKod(){
  //treba da se ide kroz sekcije i da se proverava za svaki relok zapis da li je tipa PC32 ili 32
  //ako je 32(.word) onda se uzima simbol i gleda da li je sekcija ili sam simbol
  //ako je sekcija onda se uzima pocetak sekcije, sabira sa addendom i smesta na offset iz relok zapisa(u odnosu na pocetak sekcije)
  for(Sekcija* sek = tsek->head; sek != nullptr; sek = sek->next){
    tabelaRelokZapisa* trz = sek->dohvatiTabeluRelZap();
    for(relokZapis* rz = trz->head; rz != nullptr; rz = rz->next){
      if(strcmp(rz->dohvatiTip(), "32") == 0){
        //.word je
        Simbol* sim = ts->pronadjiSimbol(rz->dohvatiSimbol());
        if(strcmp(sim->dohvatiNaziv(),sek->dohvatiNaziv()) == 0){
          //lokalni je simbol u pitanju, i zato je njegova sekcija upisana umesto samog simbola
          int novaVrednost = sek->dohvatiPocetak() + rz->dohvatiAddend();
          unsigned char c4 = novaVrednost & 0xFF;
          unsigned char c3 = (novaVrednost >> 8) & 0xFF;
          unsigned char c2 = (novaVrednost >> 16) & 0xFF;
          unsigned char c1 = (novaVrednost >> 24) & 0xFF;
          sek->masinskiKod.at(rz->dohvatiOffset()) = c1;
          sek->masinskiKod.at(rz->dohvatiOffset() + 1) = c2;
          sek->masinskiKod.at(rz->dohvatiOffset() + 2) = c3;
          sek->masinskiKod.at(rz->dohvatiOffset() + 3) = c4;

        }
        else{
          //simbol je globalni, treba da se upise njegova vrednost na offset
          int vrSimbola = sim->dohvatiVrednost();
          unsigned char c4 = vrSimbola & 0xFF;
          unsigned char c3 = (vrSimbola >> 8) & 0xFF;
          unsigned char c2 = (vrSimbola >> 16) & 0xFF;
          unsigned char c1 = (vrSimbola >> 24) & 0xFF;
          sek->masinskiKod.at(rz->dohvatiOffset()) = c1;
          sek->masinskiKod.at(rz->dohvatiOffset() + 1) = c2;
          sek->masinskiKod.at(rz->dohvatiOffset() + 2) = c3;
          sek->masinskiKod.at(rz->dohvatiOffset() + 3) = c4;
        }
      }
      else{
        //u pitanju je relativni relok zapis --> skok
        //treba sa offseta relok zapisa uzeti i procitati treci i cetvrti bajt
        //od treceg uzeti samo visa??? 4 bita
        //onda uzeti i tu vr pretvoriti u dec i sabrati sa offsetom relok zapisa
        //e na tu adr treba upisati vrednost simbola (+- addend)
        unsigned char c1 = sek->masinskiKod.at(rz->dohvatiOffset() + 2) & 0x0F;
        unsigned char c2 = sek->masinskiKod.at(rz->dohvatiOffset() + 3) & 0xFF;
        int novaVrednost = (c1 << 8) + c2;
        Simbol* sim = ts->pronadjiSimbol(rz->dohvatiSimbol());
        sek->masinskiKod.at(rz->dohvatiOffset() + novaVrednost + 4) = (sim->dohvatiVrednost() >> 24) & 0xFF;
        sek->masinskiKod.at(rz->dohvatiOffset() + novaVrednost + 4 + 1) = (sim->dohvatiVrednost() >> 16) & 0xFF;
        sek->masinskiKod.at(rz->dohvatiOffset() + novaVrednost + 4 + 2) = (sim->dohvatiVrednost() >> 8) & 0xFF;
        sek->masinskiKod.at(rz->dohvatiOffset() + novaVrednost + 4 + 3) = (sim->dohvatiVrednost() >> 0) & 0xFF;

      }
    }
  }

}

void Linker::ispisiUHexFajl(){
  ofstream file(outputfile);
  if(!file.is_open()){
    cout<<"Greska pri otvaranju fajla!"<<endl;
    return;
  }

  vector<Sekcija> sekcije;

  for(Sekcija* sek = tsek->head; sek != nullptr; sek = sek->next){
    sekcije.push_back(*sek);
  }
  sort(sekcije.begin(), sekcije.end(), Sekcija::compareByPocetak);

  for(Sekcija sek : sekcije){
    int pocetak_sek = sek.dohvatiPocetak();
    int i = 0;
    if(sek.masinskiKod.size() % 4 > 0){
      unsigned char c = 0x00;
      while(sek.masinskiKod.size() % 4 > 0){
        sek.masinskiKod.push_back(c);
      }
    }
    for(auto kod = sek.masinskiKod.begin(); kod < sek.masinskiKod.end(); kod = kod + 4){
      unsigned char c1 = *(kod+3);
      unsigned char c2 = *(kod+2);
      unsigned char c3 = *(kod+1);
      unsigned char c4 = (*kod);

      if(i==0){
        
        file << std::setw(4) << std::setfill('0') << hex << pocetak_sek << " : ";
        //printf("%08X : ", pocetak_sek);

      }
      pocetak_sek += 4;
      
      file << std::setw(2) << std::setfill('0') << hex << int(c1) << " ";
      file << std::setw(2) << std::setfill('0') << hex << int(c2) << " ";
      file << std::setw(2) << std::setfill('0') << hex << int(c3) << " ";
      file << std::setw(2) << std::setfill('0') << hex << int(c4) << " ";
      //printf("%02X ", c);
      //printf("%02X %02X %02X %02X ", c1, c2, c3, c4);
      i+=4;
      if(i == 8){
        
        file << endl;
        //printf("\n");
        i = 0;
      }
      
      
    }
    if(i != 0){
        while(i != 0){
          file << std::setw(2) << std::setfill('0') <<hex<<(int)0<<" ";
          //printf("%02X", 0);
          i++;
          i = i % 8;
        }
        file << endl;
        //printf("\n");
      }
  }

  file.close();
}



