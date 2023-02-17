CC=gcc
CFLAGS=-g -Wall -std=gnu99
OUT=sysfdtbl.out
OBJ=main.o fileDesc.o misc.o
RARGS=

$(OUT): $(OBJ)
	$(CC) $+ -o $(OUT)

run: $(OUT)
	@./$(OUT) $(RARGS)

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ) $(OUT)