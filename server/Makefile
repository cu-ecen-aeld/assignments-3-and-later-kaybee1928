# Cross-compilation prefix
CROSS_COMPILE ?=

# Compiler
CC = $(CROSS_COMPILE)gcc

# Compiler flags
CFLAGS = -Wall -Werror

# Default target
all: aesdsocket

# Default target - build executable
aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) aesdsocket.o -o aesdsocket

# Compile to object file
aesdsocket.o: aesdsocket.c aesdsocket.h
	$(CC) $(CFLAGS) -c aesdsocket.c -o aesdsocket.o

# Clean target
clean:
	rm -f *.o aesdsocket

.PHONY: clean