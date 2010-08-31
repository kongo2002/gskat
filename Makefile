include config.mk

CC=gcc

SOURCES=$(wildcard *.c)
HEADS=$(wildcard *.h)
OBJECTS=$(foreach obj,$(SOURCES:.c=.o),$(obj))

EXECUTABLE=gskat

all: ${EXECUTABLE}

.c.o:
	${CC} -c ${CFLAGS} ${CPPFLAGS} $< -o $@

${OBJECTS}: ${HEADS} Makefile

${EXECUTABLE}: ${OBJECTS}
	${CC} ${OBJECTS} ${LDFLAGS} -o $@

clean:
	rm -f ${OBJECTS}
	rm -f ${EXECUTABLE}
