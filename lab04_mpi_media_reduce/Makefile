CC = mpicc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = media_mpi
SRC = media_mpi.c
N ?= 1000
NP ?= 4

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	mpirun -np $(NP) ./$(TARGET) $(N)

clean:
	rm -f $(TARGET)
