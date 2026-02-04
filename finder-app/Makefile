# Cross-compilation prefix
CROSS_COMPILE ?=

# Compiler
CC = $(CROSS_COMPILE)gcc

# Compiler flags
CFLAGS = -Wall -Werror

# Default target - build executable
writer: writer.o
	$(CC) $(CFLAGS) writer.o -o writer

# Compile to object file
writer.o: writer.c
	$(CC) $(CFLAGS) -c writer.c -o writer.o

# Clean target
clean:
	rm -f *.o writer

.PHONY: clean