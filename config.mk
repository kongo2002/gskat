VERSION := 0.1

PKGS := gtk+-2.0
INCS := $(shell pkg-config --cflags $(PKGS))
LIBS := $(shell pkg-config --libs $(PKGS))

CPPFLAGS := -DVERSION=\"${VERSION}\" -DDEBUG ${CPPFLAGS}
CFLAGS   := -ggdb -O0 -W -Wall -Wextra ${INCS} ${CFLAGS}
LDFLAGS  := ${LIBS} ${LDFLAGS}
