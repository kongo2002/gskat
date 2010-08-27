# Get the current version which is either a git tag or a short-hash
# of the current commit
VERSION ?= $(shell build-utils/getversion.sh)

# Get includes and libs
PKGS := gtk+-2.0
INCS := $(shell pkg-config --cflags $(PKGS))
LIBS := $(shell pkg-config --libs $(PKGS))

# Define flags
CPPFLAGS := -DVERSION=\"${VERSION}\" -DDEBUG ${CPPFLAGS}
CFLAGS   := -ggdb -O0 -W -Wall -Wextra ${INCS} ${CFLAGS}
LDFLAGS  := ${LIBS} ${LDFLAGS}
