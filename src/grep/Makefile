CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
TARGET = s21_grep

SRC = main.c s21_grep.c ../common/s21_common.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean rebuild test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: all
	@echo "See tests/ at project root for the comparison test suite."

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all
