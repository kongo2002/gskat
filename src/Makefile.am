bin_PROGRAMS = gskat

INCLUDES = $(GTK_CFLAGS) \
	   -DGSKAT_LOCALEDIR=\""$(datadir)/locale"\"

LIBS = $(GTK_LIBS)

AM_CPPFLAGS=-Wall -Wextra -Wmissing-prototypes \
	    -DDEBUG \
	    -DG_DISABLE_DEPRECATED \
	    -DGTK_DISABLE_DEPRECATED \
	    -DGDK_DISABLE_DEPRECATED \
	    -DGDK_PIXBUF_DEPRECATED

gskat_headers = ai.h \
		callback.h \
		common.h \
		configuration.h \
		def.h \
		draw.h \
		game.h \
		gamestate.h \
		interface.h \
		license.h \
		null.h \
		utils.h

gskat_SOURCES = ai.c \
		callback.c \
		common.c \
		configuration.c \
		draw.c \
		game.c \
		gamestate.c \
		interface.c \
		main.c \
		null.c \
		utils.c \
		$(gskat_headers)
