all:
	@gcc -std=c99 -g main.c -Wall \
	-Wcast-qual -Wcast-align -Wwrite-strings -Winline -Wredundant-decls -Wpointer-arith \
	&& ./a.out && echo ''

build:
	@gcc -std=c99 -g main.c -Wall \
	-Wcast-qual -Wcast-align -Wwrite-strings -Winline -Wredundant-decls -Wpointer-arith \
	&& echo ''

clean:
	@rm -rf *.o a.out