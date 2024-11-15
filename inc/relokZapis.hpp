#ifndef RELOKZAPIS_HPP
#define RELOKZAPIS_HPP


class relokZapis{
  private:
    int id;
    int offset;
    char* tip;
    char* simbol;
    int addend;
    int sekcija;
    

  public:
  static int prev_id;
  relokZapis* next;
    relokZapis();
    ~relokZapis();
    void postaviOffset(int o);
    void postaviTip(char* t);
    void postaviSimbol(char* s);
    void postaviAddend(int a);
    int dohvatiOffset();
    char* dohvatiTip();
    char* dohvatiSimbol();
    int dohvatiAddend();
    void postaviSekciju(int s);
    int dohvatiSekciju();
    int dohvatiId();
};



#endif