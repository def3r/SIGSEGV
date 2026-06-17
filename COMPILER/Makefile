.PHONY: all clean

all: anbn palindrome balancedParan for2while validmail

# anbn
anbn: anbn.tab.c anbn.l
	flex anbn.l
	gcc anbn.tab.c lex.yy.c -o anbn.out

anbn.tab.c:
	bison -d anbn.y

# Palindrome
palindrome: palindrome.tab.c lex.yy.c
	gcc palindrome.tab.c lex.yy.c -o palindrome.out

palindrome.tab.c palindrome.tab.h: palindrome.y
	bison -d palindrome.y

lex.yy.c: palindrome.l palindrome.tab.h
	lex palindrome.l

# balancedParan
balancedParan: balancedParan.tab.c balancedParan.l
	flex balancedParan.l
	gcc balancedParan.tab.c lex.yy.c -o balancedParan.out

balancedParan.tab.c:
	bison -d balancedParan.y

# for 2 while
for2while: for2while.l
	bison -d for2while.y
	flex for2while.l
	gcc for2while.tab.c lex.yy.c -o for2while.out

validmail: validmail.l
	flex validmail.l
	gcc lex.yy.c -o validmail.out

even0s: even0s.y even0s.l
	bison -d even0s.y
	flex even0s.l
	gcc even0s.tab.c lex.yy.c -o even0s.out

sdt: sdt_parser.tab.c sdt_lexer.l
	flex sdt_lexer.l
	gcc sdt_parser.tab.c lex.yy.c -o sdt.out

sdt_parser.tab.c:
	bison -d sdt_parser.y

inf2pof: inf2pof.tab.c inf2pof.l
	bison -d inf2pof.y
	flex inf2pof.l
	gcc inf2pof.tab.c lex.yy.c -o inf2pof.out

inf2pof.tab.c:
	bison -d inf2pof.y

redcount: redcount.y redcount.l
	bison -d redcount.y
	flex redcount.l
	gcc redcount.tab.c lex.yy.c -o redcount.out

ptht: ptht.y ptht.l
	bison -d ptht.y
	flex ptht.l
	gcc ptht.tab.c lex.yy.c -o ptht.out

binstr1: binstr1.y binstr1.l
	bison -d binstr1.y
	flex binstr1.l
	gcc binstr1.tab.c lex.yy.c -o binstr1.out

clean:
	rm -rf lex.yy.c
	rm -rf even0s.tab.c for2while.tab.c palindrome.tab.c balancedParan.tab.c anbn.tab.c
	rm -rf even0s.tab.h for2while.tab.h palindrome.tab.h balancedParan.tab.h anbn.tab.h
	rm -rf even0s.out   for2while.out   palindrome.out   balancedParan.out   anbn.out
	rm -rf validmail.out
