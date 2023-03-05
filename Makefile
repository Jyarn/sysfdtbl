CC=gcc
CFLAGS=-g -Wall -std=gnu99
OUT=sysfdtbl.out
OBJ=main.o fileDesc.o misc.o IO.o
RARGS=

$(OUT): $(OBJ)
	$(CC) $+ -o $(OUT)


run: $(OUT)
	./$(OUT) $(RARGS) | less

gdb: $(OUT)
	gdb --args $(OUT) $(RARGS)

vl: $(OUT)
	valgrind --leak-check=full --track-origins=yes -s ./$(OUT) $(RARGS)

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm $(OBJ) $(OUT)
	rm compositeTable.*
