build: main.c
	gcc -std=c99 -D_POSIX_C_SOURCE=200112L main.c

clean:
	$(RM) ./a.out
