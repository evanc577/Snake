CC = gcc
CFLAGS = -lncurses -lpthread -Wall -O3
EXE = snake

all: snake.c snake.h list.h
	$(CC) $(CFLAGS) $^ -o $(EXE)
