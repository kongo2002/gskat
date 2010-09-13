bin_PROGRAMS = gskat

INCLUDES = $(GTK_CFLAGS) \
	   -DGSKAT_LOCALEDIR=\""$(datadir)/locale"\"

LIBS = $(GTK_LIBS)

AM_CPPFLAGS=-Wall -Wextra -Wmissing-prototypes -DDEBUG

gskat_headers = ai.h \
		callback.h \
		configuration.h \
		def.h \
		game.h \
		gamestate.h \
		interface.h \
		license.h \
		utils.h

gskat_SOURCES = ai.c \
		callback.c \
		configuration.c \
		game.c \
		gamestate.c \
		interface.c \
		main.c \
		utils.c \
		$(gskat_headers)
