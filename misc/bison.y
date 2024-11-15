/* If we want to use other functions, we have to put the relevant
 * header includes here. */
%{
	#include <stdio.h>
	#include <string.h>
  #include "../inc/instrukcija.hpp"
	#include <cstring>
	int yylex(void);
	extern int yylex(void);
	extern FILE* yyin;
	extern void yyerror(const char*);
%}

/* These declare our output file names. */
%output "misc/parser.cpp"
%defines "misc/parser.hpp"

/* This union defines the possible return types of both lexer and
 * parser rules. We'll refer to these later on by the field name */
%union {
	int         num;
	char       *ident;
	struct operand* arg;
}

/* These define the tokens that we use in the lexer.
 * All of these have no meaningful return value. */
%token TOKEN_LPAR
%token TOKEN_RPAR
%token TOKEN_PLUS
%token TOKEN_SEMI
%token TOKEN_COMMA
%token TOKEN_DOLAR
%token TOKEN_PERCENT
%token TOKEN_LUPAR
%token TOKEN_RUPAR

/* These are ALSO used in the lexer, but in addition to
 * being tokens, they also have return values associated
 * with them. We name those according to the names we used
 * above, in the %union declaration. So, the TOKEN_NUM
 * rule will return a value of the same type as num, which
 * (in this case) is an int. */
%token <num>   TOKEN_NUM
%token <ident> TOKEN_HALT
%token <ident> TOKEN_INT
%token <ident> TOKEN_IRET
%token <ident> TOKEN_CALL
%token <ident> TOKEN_RET
%token <ident> TOKEN_JMP
%token <ident> TOKEN_BEQ
%token <ident> TOKEN_BNE
%token <ident> TOKEN_BGT
%token <ident> TOKEN_PUSH
%token <ident> TOKEN_POP
%token <ident> TOKEN_XCHG
%token <ident> TOKEN_ADD
%token <ident> TOKEN_SUB
%token <ident> TOKEN_MUL
%token <ident> TOKEN_DIV
%token <ident> TOKEN_NOT
%token <ident> TOKEN_AND
%token <ident> TOKEN_OR
%token <ident> TOKEN_XOR
%token <ident> TOKEN_SHL
%token <ident> TOKEN_SHR
%token <ident> TOKEN_LD
%token <ident> TOKEN_ST
%token <ident> TOKEN_CSRRD
%token <ident> TOKEN_CSRWR
%token <ident> TOKEN_SIMBOL

%token <ident> TOKEN_GLOBAL
%token <ident> TOKEN_EXTERN
%token <ident> TOKEN_SECTION
%token <ident> TOKEN_WORD
%token <ident> TOKEN_SKIP
%token <ident> TOKEN_END

%token <ident> TOKEN_REGISTER
%token <ident> TOKEN_SREGISTER
%token <ident> TOKEN_LABEL

/* These are non-terminals in our grammar, by which I mean, parser
 * rules down below. Each of these also has a meaningful return type,
 * which is declared in the same way. */
%type <arg> operand
%type <arg> lista
%type <arg> listasimbola

%%

prog:
| line prog 
;

line: direktiva 
| instrukcija
| TOKEN_LABEL {
	ubaciLabelu($1);
}
;

direktiva: TOKEN_GLOBAL listasimbola {
	ubaciDirektivuSaListom($1, $2);
}
| TOKEN_EXTERN listasimbola {
	ubaciDirektivuSaListom($1, $2);
}
| TOKEN_SECTION TOKEN_SIMBOL {
	struct operand* op = napraviOperand(-1, $2, Adresiranje::MEMDIR, NULL);
	ubaciDirektivuBezListe($1, op);
}
| TOKEN_WORD lista {
	ubaciDirektivuSaListom($1, $2);
}
| TOKEN_SKIP TOKEN_NUM {
	struct operand* op = napraviOperandKojiSadrziLiteral(-1, $2, Adresiranje::MEMDIR);
	ubaciDirektivuBezListe($1, op);
}
| TOKEN_END {
	ubaciDirektivuBezListe($1, NULL);
}
; 

lista: TOKEN_SIMBOL TOKEN_COMMA lista {
	struct operand* op = napraviOperand(-1, $1, Adresiranje::MEMDIR, NULL);
	struct operand* list = (struct operand*)($3);
	op->next = list;
	$$ = op;
}
| TOKEN_SIMBOL {
	$$ = napraviOperand(-1, $1, Adresiranje::MEMDIR, NULL);
}
| TOKEN_NUM TOKEN_COMMA lista {
	struct operand* op = napraviOperandKojiSadrziLiteral(-1, $1, Adresiranje::MEMDIR);
	struct operand* list = (struct operand*)($3);
	op->next = list;
	$$ = op;
}
| TOKEN_NUM {
	$$ = napraviOperandKojiSadrziLiteral(-1, $1, Adresiranje::MEMDIR);
}
;

listasimbola: TOKEN_SIMBOL TOKEN_COMMA listasimbola {
	struct operand* op = napraviOperand(-1, $1, Adresiranje::MEMDIR, NULL);
	struct operand* list = (struct operand*)($3);
	op->next = list;
	$$ = op;
} 
| TOKEN_SIMBOL {
	$$ = napraviOperand(-1, $1, Adresiranje::MEMDIR, NULL);
}
;


instrukcija: TOKEN_HALT { ubaciInstrukciju($1, NULL, NULL, NULL); } 
| TOKEN_INT { ubaciInstrukciju($1, NULL, NULL, NULL); } 
| TOKEN_IRET { ubaciInstrukciju($1, NULL, NULL, NULL); } 
| TOKEN_CALL operand { ubaciInstrukciju($1, $2, NULL, NULL); } 
| TOKEN_RET { ubaciInstrukciju($1, NULL, NULL, NULL); } 
| TOKEN_JMP operand { ubaciInstrukciju($1, $2, NULL, NULL); }
| TOKEN_BEQ TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER TOKEN_COMMA operand { 
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
  ubaciInstrukciju($1, op1, op2, $6);
  }
| TOKEN_BNE TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER TOKEN_COMMA operand {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, $6);
}
| TOKEN_BGT TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER TOKEN_COMMA operand {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, $6);
}
| TOKEN_PUSH TOKEN_REGISTER { 
	int regbr;
	if(strcmp($2, "%sp") == 0){
		regbr = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr = 15;
	}
	else{
	regbr = (int)atoi($2 + 2);
	}
	struct operand* op = napraviOperand(regbr, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op, NULL, NULL);
}
| TOKEN_POP TOKEN_REGISTER {
	int regbr;
	if(strcmp($2, "%sp") == 0){
		regbr = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr = 15;
	}
	else{
	regbr = (int)atoi($2 + 2);
	}
	struct operand* op = napraviOperand(regbr, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op, NULL, NULL);
}
| TOKEN_XCHG TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_ADD TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_SUB TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_MUL TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_DIV TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_NOT TOKEN_REGISTER {
		int regbr;
	if(strcmp($2, "%sp") == 0){
		regbr = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr = 15;
	}
	else{
	regbr = (int)atoi($2 + 2);
	}
	struct operand* op = napraviOperand(regbr, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op, NULL, NULL);
}
| TOKEN_AND TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_OR TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_XOR TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_SHL TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_SHR TOKEN_REGISTER TOKEN_COMMA TOKEN_REGISTER {
		int regbr1;
	if(strcmp($2, "%sp") == 0){
		regbr1 = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr1 = 15;
	}
	else{
	regbr1 = (int)atoi($2 + 2);
	}
		int regbr2;
	if(strcmp($4, "%sp") == 0){
		regbr2 = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr2 = 15;
	}
	else{
	regbr2 = (int)atoi($4 + 2);
	}
	struct operand* op1 = napraviOperand(regbr1, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(regbr2, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_LD operand TOKEN_COMMA TOKEN_REGISTER {
	//ld [%sp + 0x08], %r1
			int regbr;
	if(strcmp($4, "%sp") == 0){
		regbr = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr = 15;
	}
	else{
	regbr = (int)atoi($4 + 2);
	}
	struct operand* op = napraviOperand(regbr, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, $2, op, NULL);
}
| TOKEN_ST TOKEN_REGISTER TOKEN_COMMA operand {
			int regbr;
	if(strcmp($2, "%sp") == 0){
		regbr = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr = 15;
	}
	else{
	regbr = (int)atoi($2 + 2);
	}
	struct operand* op = napraviOperand(regbr, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op, $4, NULL);
}
| TOKEN_CSRRD TOKEN_SREGISTER TOKEN_COMMA TOKEN_REGISTER {
	struct operand* op1 = napraviOperand(-1, NULL, Adresiranje::REGDIR, $2);
		int regbr;
	if(strcmp($4, "%sp") == 0){
		regbr = 14;
	}else if(strcmp($4, "%pc") == 0){
		regbr = 15;
	}
	else{
	regbr = (int)atoi($4 + 2);
	}
	struct operand* op2 = napraviOperand(regbr, NULL, Adresiranje::REGDIR, NULL);
	ubaciInstrukciju($1, op1, op2, NULL);
}
| TOKEN_CSRWR TOKEN_REGISTER TOKEN_COMMA TOKEN_SREGISTER {
		int regbr;
	if(strcmp($2, "%sp") == 0){
		regbr = 14;
	}else if(strcmp($2, "%pc") == 0){
		regbr = 15;
	}
	else{
	regbr = (int)atoi($2 + 2);
	}
	struct operand* op1 = napraviOperand(regbr, NULL, Adresiranje::REGDIR, NULL);
	struct operand* op2 = napraviOperand(-1, NULL, Adresiranje::REGDIR, $4);
	ubaciInstrukciju($1, op1, op2, NULL);
}
;


operand: TOKEN_DOLAR TOKEN_NUM {
	$$ = napraviOperandKojiSadrziLiteral(-1, $2, Adresiranje::IMMED); 
}
| TOKEN_DOLAR TOKEN_SIMBOL {
	$$ = napraviOperand(-1, $2, Adresiranje::IMMED, NULL); 
}
| TOKEN_NUM {
	$$ = napraviOperandKojiSadrziLiteral(-1, $1, Adresiranje::MEMDIR); 
}
| TOKEN_SIMBOL {
	$$ = napraviOperand(-1, $1, Adresiranje::MEMDIR, NULL); 
}
| TOKEN_REGISTER {
	int regbr;
	if(strcmp($1, "%sp") == 0){
		regbr = 14;
	}else if(strcmp($1, "%pc") == 0){
		regbr = 15;
	}
	else{
	regbr = (int)atoi($1 + 2);
	}
	
	$$ = napraviOperand(regbr, NULL, Adresiranje::REGDIR, NULL); 
}
| TOKEN_LUPAR TOKEN_REGISTER TOKEN_RUPAR {
	int regbr;
	if(strcmp($2, "%sp") == 0){
		regbr = 14;
	}
	else if(strcmp($2, "%pc") == 0){
		regbr = 15;
	}
	else{
		regbr = (int)atoi($2 + 2);
	}
	$$ = napraviOperand(regbr, NULL, Adresiranje::REGIND, NULL); 
}
| TOKEN_LUPAR TOKEN_REGISTER TOKEN_PLUS TOKEN_NUM TOKEN_RUPAR {
	//[%sp + 0x08]
	int regbr;
	if(strcmp($2, "%sp") == 0){
		regbr = 14;
	}
	else if(strcmp($2, "%pc") == 0){
		regbr = 15;
	}
	else{
		regbr = (int)atoi($2 + 2);
	}
	$$ = napraviOperandKojiSadrziLiteral(regbr, $4, Adresiranje::REGINDPOM); 
}
| TOKEN_LUPAR TOKEN_REGISTER TOKEN_PLUS TOKEN_SIMBOL TOKEN_RUPAR {
	int regbr;
	if(strcmp($2, "%sp") == 0){
		regbr = 14;
	}
	else if(strcmp($2, "%pc") == 0){
		regbr = 15;
	}
	else{
		regbr = (int)atoi($2 + 2);
	}
	$$ = napraviOperand(regbr, $4, Adresiranje::REGINDPOM, NULL); 
}
;

/* A program is defined recursively as either empty or an instruction
 * followed by another program. In this case, there's nothing meaningful
 * for us to do or return as an action, so we omit any action after the
 * rules. */

/* An instruction, in our toy assembly, is always an identifier (which
 * is the instruction name) and possibly arguments. The numbers in the
 * variable here refer to the position of the argument we want, and
 * will refer to either the result of the rule (in the case of other
 * parser rules) or the contents of yylval (in the case of lexer
 * tokens.) */



/* An argument in this case has multiple choices: it can be a register
 * plus an offset, in which case it must be surrounded by parens, or
 * it can be just a register, in which case the parens are optional.
 *
 * The 'return value' of a rule is always stored in the $$ variable.
 * Here, I also name the results of terminals and non-terminals, instead
 * of addressing them by number.
 */


/* In this case, I'm being a little bit obtuse: I should encode the
 * register name rule as a regular expression (i.e. all registe
 rs could
 * be identified by the regex r.*x) but instead I'm doing a check in the
 * body of the rule and returning out of the generated parser. */


%%