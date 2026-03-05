CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
TARGET := atividade
SRC := atividade_01_daniela_10395891_marcello_10381938.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
