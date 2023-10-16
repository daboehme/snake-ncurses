CC      = gcc
LINK    = gcc

CFLAGS  = -W -Wall -O2
LFLAGS  = -lncurses -ltinfo -lrt

TARGET  = snake
OBJECTS = snake.o

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

all: ${TARGET}

$(TARGET): $(OBJECTS)
	$(LINK) -o $@ $(LFLAGS) $<

clean:
	rm -f ${OBJECTS} ${TARGET}

.PHONY: all clean