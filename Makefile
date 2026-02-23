mac:
	gcc -Wall -Wextra -g -o main \
		-I/opt/homebrew/include \
    -L/opt/homebrew/lib \
		-lraylib -lm \
		main.c
	./main

linux:
	gcc -Wall -Wextra -g -o main -lraylib -lm main.c
	./main

win:
	x86_64-w64-mingw32-gcc -Wall -Wextra -g -o main -lraylib -lm main.c
	./main
