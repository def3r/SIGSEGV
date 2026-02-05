.PHONY: all clean

all: anbn palindrome balancedParan for2while

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

# for 2 while
for2while: for2while.l
	bison -d for2while.y
	flex for2while.l
	gcc for2while.tab.c lex.yy.c -o for2while.out

clean:
	rm -rf lex.yy.c
	rm -rf for2while.tab.c palindrome.tab.c balancedParan.tab.c anbn.tab.c
	rm -rf for2while.tab.h palindrome.tab.h balancedParan.tab.h anbn.tab.h
	rm -rf for2while.out   palindrome.out   balancedParan.out    anbn.out
