bin_PROGRAMS = gskat

INCLUDES = $(GTK_CFLAGS) \
	   -DGSKAT_LOCALEDIR=\""$(datadir)/locale"\"

LIBS = $(GTK_LIBS)

gskat_CPPFLAGS=$(AM_CPPFLAGS) \
	       -Wall -Wextra -Wmissing-prototypes \
	       -DG_DISABLE_DEPRECATED \
	       -DG_DISABLE_SINGLE_INCLUDES \
	       -DGTK_DISABLE_DEPRECATED \
	       -DGTK_DISABLE_SINGLE_INCLUDES \
	       -DGDK_DISABLE_DEPRECATED \
	       -DGDK_DISABLE_SINGLE_INCLUDES \
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
