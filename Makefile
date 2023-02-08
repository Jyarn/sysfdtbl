CC=gcc
CFLAGS=-g -Wall -std=c99
OUT=sysfdtbl.out
OBJ=main.o
RARGS=

build: $(OBJ)
	$(CC) $+ -o $(OUT)

run: build
	./$(OUT) $(RARGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ)