.PHONY: all clean

all: anbn palindrome balancedParan for2while validmail

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

validmail: validmail.l
	flex validmail.l
	gcc lex.yy.c -o validmail.out

even0s: even0s.y even0s.l
	bison -d even0s.y
	flex even0s.l
	gcc even0s.tab.c lex.yy.c -o even0s.out

clean:
	rm -rf lex.yy.c
	rm -rf even0s.tab.c for2while.tab.c palindrome.tab.c balancedParan.tab.c anbn.tab.c
	rm -rf even0s.tab.h for2while.tab.h palindrome.tab.h balancedParan.tab.h anbn.tab.h
	rm -rf even0s.out   for2while.out   palindrome.out   balancedParan.out   anbn.out
	rm -rf validmail.out
