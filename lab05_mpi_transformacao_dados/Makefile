CC = mpicc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = transformacao_dados_mpi
SRC = transformacao_dados_mpi.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	mpirun -np 5 ./$(TARGET)

clean:
	rm -f $(TARGET)
