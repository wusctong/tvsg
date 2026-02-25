unix:
	gcc -Wall -Wextra -g -o main \
		-I/opt/homebrew/include \
    -L/opt/homebrew/lib \
		-lraylib -lm \
		main.c
	./main
	rm -rf ./main.dSYM ./main ./DS_Store

win:
	x86_64-w64-mingw32-gcc -Wall -Wextra -g -o main \
		-I../env/win64/include \
	  -L../env/win64/lib \
		-lraylib -lm \
		main.c
	./main.exe
