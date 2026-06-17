all: switch2if.out

switch2if.tab.c switch2if.tab.h: switch2if.y
	bison -d -o switch2if.tab.c switch2if.y

lex.yy.c: switch2if.l switch2if.tab.h
	flex -o lex.yy.c switch2if.l

switch2if.out: switch2if.tab.c lex.yy.c
	gcc -o switch2if.out switch2if.tab.c lex.yy.c

clean:
	rm -f switch2if.out switch2if.tab.c switch2if.tab.h lex.yy.c
