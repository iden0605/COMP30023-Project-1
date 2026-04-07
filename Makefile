EXE=schedule
CC=cc
CFLAGS=-O3 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
SRC=main.c process.c queue.c scheduler.c
OBJ=$(SRC:.c=.o)
HDR=process.h queue.h scheduler.h

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

format:
	clang-format -style=file -i *.c *.h

clean:
	rm -f $(EXE) $(OBJ)

.PHONY: format clean
