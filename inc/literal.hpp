#ifndef LITERAL_HPP
#define LITERAL_HPP

struct flinkLiteral{
    struct Instrukcija* instrukcija;
    int lc;
    int sectionNumber;
    flinkLiteral* next;
};

class Literal{
  private:
    int vrednost;
    char* simbol;
    int pozicijaUTabeli;
    int id;
    bool daLiJeSimbol;
    
  public:
    struct flinkLiteral* headFlinkLiteral;
    struct flinkLiteral* tailFlinkLiteral;
  Literal* next;
    Literal();
    ~Literal();
    static int prev_id;
    void postaviVrednost(int v);
    void postaviId(int i);
    int dohvatiId();
    int dohvatiVrednost();
    void postaviNext(Literal* l);
    void postaviPozicijuUTabeli(int p);
    int dohvatiPozicijuUTabeli();
    bool dohvatiDaLiJeSimbol();
    char* dohvatiSimbol();
    void postaviSimbol(char* s);
    void postaviDaLiJeSimbol(bool b);

    void ubaciFlinkInstrukciju(struct Instrukcija* ins, int lc, int sn);
};



#endif