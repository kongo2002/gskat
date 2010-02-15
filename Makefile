CC=gcc
CFLAGS=-c -O0 -Wall `pkg-config --cflags gtk+-2.0` -g -DDEBUG
LDFLAGS=`pkg-config --libs gtk+-2.0`
SOURCES=main.c interface.c callback.c game.c utils.c ai.c
OBJECTS=$(SOURCES:.c=.o)
INCLUDES=
EXECUTABLE=gskat

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(INCLUDES) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm $(OBJECTS)
	rm $(EXECUTABLE)
