.PHONY: all clean

all: anbn palindrome balancedParan

# anbn
anbn: anbn.tab.c anbn.l
	flex anbn.l
	gcc anbn.tab.c lex.yy.c -o anbn.out

anbn.tab.c:
	bison -d anbn.y

# Palindrome
palindrome: palindrome.tab.c palindrome.l
	flex palindrome.l
	gcc palindrome.tab.c lex.yy.c -o palindrome.out

palindrome.tab.c:
	bison -d palindrome.y

# balancedParan
balancedParan: balancedParan.tab.c balancedParan.l
	flex balancedParan.l
	gcc balancedParan.tab.c lex.yy.c -o balancedParan.out

balancedParan.tab.c:
	bison -d balancedParan.y

clean:
	rm -rf lex.yy.c
	rm -rf palindrome.tab.c balancedParan.tab.c anbn.tab.c
	rm -rf palindrome.tab.h balancedParan.tab.h anbn.tab.h
	rm -rf palindrome.out balancedParan.out anbn.out
