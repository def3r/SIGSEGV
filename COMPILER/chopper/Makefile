FILE := a.out

.PHONY: all clean

all:
	lex lexer.l
	gcc lex.yy.c -o $(FILE)
	cat progfile.txt | ./$(FILE)

clean:
	rm -rf lex.yy.c
	rm -rf $(FILE)

