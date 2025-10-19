# Simple Makefile for myshell

CC = gcc
CFLAGS = -Iinclude -Wall
SRC = src/main.c src/shell.c src/execute.c
OBJ = $(SRC:.c=.o)
BIN = bin/myshell

all: $(BIN)

$(BIN): $(OBJ)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

clean:
	rm -rf $(OBJ) bin

