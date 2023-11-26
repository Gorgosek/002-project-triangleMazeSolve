CC=gcc
CFLAGS=-std=c11 -Wall -Wextra 
DEBUGFLAGS=-g
TARGET=maze
SOURCE=maze.c
TESTSCRIPT=./maze-test.sh

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

debug: $(SOURCE)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(SOURCE) -o $(TARGET)

test: $(TARGET)
	$(TESTSCRIPT)

clean:
	rm -f $(TARGET)

.PHONY: all clean debug test
