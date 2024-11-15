
struct flinkInstrukcija{
    struct Instrukcija* instrukcija;
    int lc;
    int sectionNumber;
    flinkInstrukcija* next;
  };
class Simbol{

  friend class tabelaSimbola;

private:
  char* naziv;
  int vrednost;
  int size;
  int sectionNum;
  bool globalBool;
  bool externBool;
  bool sectionBool;
  char* nazivFajla;
  
  
  

  
public:
  Simbol* next;
  static int prev_id;
  int id;
  struct flinkInstrukcija* headFlink;
  struct flinkInstrukcija* tailFlink;
  bool definisan;
  

  Simbol();

  ~Simbol();

  void postaviNazivFajla(char* f){
    nazivFajla = f;
  }

  char* dohvatiNazivFajla(){
    return nazivFajla;
  }

  void postaviNaziv(char* n);

  void postaviVrednost(int v);

  int dohvatiVrednost();

  void postaviSize(int s);

  void postaviSectionNum(int s);

  int dohvatiIdSimbola();

  void postaviGlobalBool(bool b);

  void postaviExternBool(bool b);

  void postaviSectionBool(bool b);

  bool dohvatiSectionBool();

  void ubaciFlinkInstrukciju(struct Instrukcija* ins, int lc, int sn);

  struct Instrukcija* dohvatiFlinkInstrukciju();

  void postaviDefinisanost(bool d);

  bool dohvatiDefinisanost();

  bool dohvatiGlobalBool();

  int dohvatiSectionNum();

  bool dohvatiExternBool();

  char* dohvatiNaziv();


};