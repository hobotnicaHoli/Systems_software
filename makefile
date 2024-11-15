ASSEMBLER = src/main.cpp \
	misc/parser.cpp \
	misc/lexer.cpp \
	src/instrukcija.cpp \
	src/asembler.cpp \
	src/tabelaSimbola.cpp \
	src/simbol.cpp \
	src/sekcija.cpp \
	src/tabelaSekcija.cpp \
	src/literal.cpp \
	src/tabelaLiterala.cpp \
	src/relokZapis.cpp \
	src/tabelaRelokZapisa.cpp \
	
MAINLINKER = src/mainlinker.cpp \
	src/linker.cpp \
	src/tabelaLiterala.cpp \
	src/tabelaSimbola.cpp \
	src/simbol.cpp \
	src/sekcija.cpp \
	src/tabelaSekcija.cpp \
	src/relokZapis.cpp \
	src/tabelaRelokZapisa.cpp \
	src/literal.cpp \

MAINEMULATOR = src/mainemulator.cpp \
	src/emulator.cpp \


all: tests/assembler flex bison tests/linker tests/emulator

bison: 
	bison -d misc/bison.y 

flex: bison
	flex misc/flex.l

tests/assembler: flex
	g++ -g -Iinc -o ${@} ${ASSEMBLER}

tests/linker:
	g++ -g -Iinc -o ${@} ${MAINLINKER}

tests/emulator:
	g++ -g -Iinc -o ${@} ${MAINEMULATOR}

cleanLinker:
	rm -f ./tests/linker

cleanAssembler:
	rm -f ./tests/assembler

cleanEmulator:
	rm -f ./tests/emulator