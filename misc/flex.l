/* this will include some C code directly, so we can pull in the
 * header files we want */
%{
  #include "parser.hpp"
  #include <string.h>
  #include <stdio.h>
  #include <stdlib.h>
  extern FILE *yyin;
%}

/* this tells us that we're going to output our lexer implementation
 * to `lexer.c` and the associated header to `lexer.h`.
 */
%option outfile="misc/lexer.cpp" header-file="misc/lexer.hpp"

/* the int yywrap(void) is used internally by the lexer for the case
 *  where you want to parse multiple files in sequence with the same
 * parser state. Usually, you'd supply your own implementation that
 * would set up the next input stream and let the lexer know whether
 * to keep going. In our case, we disable it; this is effectively
 * the same as a dummy implementation like #define yywrap() 0.
 */
%option noyywrap

/* these define a few literals to use later on */
LPAR  "("
RPAR  ")"
PLUS  "+"
SEMI  ";"
COMMA ","
DOLAR "$"
LUPAR "["
RUPAR "]"
PERCENT "%"

/* and these define some regular expressions we'll use. We could
 * also use these inline down below. */
HEX   0[Xx][0-9A-Fa-f]+
DEC   ([1-9][0-9]*)|0
BIN   0[Bb][0-1]+
IDENT [A-Za-z_][A-Za-z0-9_]*
COM   "#"[^\n\r]*
WS    [ \r\n\t]*
REGISTER %(r[0-9]|r1[0-5])|%sp|%pc
DIRECTIVE \.[A-Za-z_][A-Za-z0-9_]*
LABEL  [A-Za-z_][A-Za-z0-9_]*:
STATUS %status
HANDLER %handler
CAUSE %cause


/* In the below code, the TOKEN_blah types and the yylval
 * structure are used; these are defined in parser.y */

%%

{WS}        { /* skip whitspace */ }
{COM}       { /* also skip comments */ }
{HEX}		    {
  int dec = (int)strtol(yytext+2,NULL,16);
  yylval.num = dec;
  //treba da se upise
  
  return TOKEN_NUM;
}
{DEC}       {
  int dec = (int)strtol(yytext,NULL,10);
  yylval.num = dec;
  //treba da se upise
  return TOKEN_NUM;
}
{BIN}       {
  int dec = (int)strtol(yytext+2,NULL,2);
  yylval.num = dec;
  //treba da se upise
  return TOKEN_NUM;
}
{IDENT}     {
  yylval.ident = strdup(yytext);
  if(strcmp("halt",yytext) == 0){
    return TOKEN_HALT;
  }else if(strcmp("int",yytext) == 0){
    return TOKEN_INT;
  }else if(strcmp("iret",yytext) == 0){
    return TOKEN_IRET;
  }else if(strcmp("call",yytext) == 0){
    return TOKEN_CALL;
  }else if(strcmp("ret",yytext) == 0){
    return TOKEN_RET;
  }else if(strcmp("jmp",yytext) == 0){
    return TOKEN_JMP;
  }else if(strcmp("beq",yytext) == 0){
    return TOKEN_BEQ;
  }else if(strcmp("bne",yytext) == 0){
    return TOKEN_BNE;
  }else if(strcmp("bgt",yytext) == 0){
    return TOKEN_BGT;
  }else if(strcmp("push",yytext) == 0){
    return TOKEN_PUSH;
  }else if(strcmp("pop",yytext) == 0){
    return TOKEN_POP;
  }else if(strcmp("xchg",yytext) == 0){
    return TOKEN_XCHG;
  }else if(strcmp("add",yytext) == 0){
    return TOKEN_ADD;
  }else if(strcmp("sub",yytext) == 0){
    return TOKEN_SUB;
  }else if(strcmp("mul",yytext) == 0){
    return TOKEN_MUL;
  }else if(strcmp("div",yytext) == 0){
    return TOKEN_DIV;
  }else if(strcmp("not",yytext) == 0){
    return TOKEN_NOT;
  }else if(strcmp("and",yytext) == 0){
    return TOKEN_AND;
  }else if(strcmp("or",yytext) == 0){
    return TOKEN_OR;
  }else if(strcmp("xor",yytext) == 0){
    return TOKEN_XOR;
  }else if(strcmp("shl",yytext) == 0){
    return TOKEN_SHL;
  }else if(strcmp("shr",yytext) == 0){
    return TOKEN_SHR;
  }else if(strcmp("ld",yytext) == 0){
    return TOKEN_LD;
  }else if(strcmp("st",yytext) == 0){
    return TOKEN_ST;
  }else if(strcmp("csrrd",yytext) == 0){
    return TOKEN_CSRRD;
  }else if(strcmp("csrwr",yytext) == 0){
    return TOKEN_CSRWR;
  }else{
    return TOKEN_SIMBOL;
  }
  
}

{REGISTER} {
  
  yylval.ident = strdup(yytext);
  return TOKEN_REGISTER;
}
{LABEL}  {
  yylval.ident = (char*)malloc(strlen(yytext));
  strncpy(yylval.ident, yytext, strlen(yytext)-1);
  return TOKEN_LABEL;
}

{STATUS} {
  yylval.ident = strdup(yytext+1);
  return TOKEN_SREGISTER;
}
{CAUSE} {
  yylval.ident = strdup(yytext+1);
  return TOKEN_SREGISTER;
}
{HANDLER} {
  yylval.ident = strdup(yytext+1);
  return TOKEN_SREGISTER;
}

{DIRECTIVE} {
  if(strcmp(".global",yytext) == 0){
    yylval.ident = strdup(yytext+1);
    return TOKEN_GLOBAL;
  }else if(strcmp(".extern",yytext) == 0){
    yylval.ident = strdup(yytext+1);
    return TOKEN_EXTERN;
  }else if(strcmp(".section",yytext) == 0){
    yylval.ident = strdup(yytext+1);
    return TOKEN_SECTION;
  }else if(strcmp(".word",yytext) == 0){
    yylval.ident = strdup(yytext+1);
    return TOKEN_WORD;
  }else if(strcmp(".skip",yytext) == 0){
    yylval.ident = strdup(yytext+1);
    return TOKEN_SKIP;
  }else if(strcmp(".end",yytext) == 0){
    yylval.ident = strdup(yytext+1);
    return TOKEN_END;
  }else{
   printf("Nepoznata direktiva\n");
   yyterminate();
    //TODO
  }
  
}

{LPAR}      { 
return TOKEN_LPAR; 
}
{RPAR}      { 
return TOKEN_RPAR; 
}
{PLUS}      { 
return TOKEN_PLUS; 
}
{SEMI}      { 
return TOKEN_SEMI; 
}
{COMMA}     { 
return TOKEN_COMMA; 
}
{DOLAR}    { 
return TOKEN_DOLAR; 
}
{PERCENT}   { 
return TOKEN_PERCENT; 
}
{LUPAR}     { 
return TOKEN_LUPAR; 
}
{RUPAR}     { 
return TOKEN_RUPAR; 
}

%%

/* In the event of some kind of error, this will get called. */
int
yyerror(const char* msg)
{
    fprintf(stderr, "lexer error: %s\n", msg);
    return 0;
}