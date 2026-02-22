linux:
	gcc -Wall -Wextra -g -o main -lraylib -lm main.c
	./main

win:
	x86_64-w64-mingw32-gcc -Wall -Wextra -g -o main -lraylib -lm main.c
	./main
