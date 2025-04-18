all: main.c
	gcc main.c -o main -lncurses -lcjson -lmenu -lpanel -lm -g 
