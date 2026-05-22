CC = mpicc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = soma_quadrados
SRC = soma_quadrados.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	mpirun -np 4 ./$(TARGET)

clean:
	rm -f $(TARGET)
