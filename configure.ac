AC_PREREQ([2.59])

# Initial project information
AC_INIT([gskat], [0.1], [kongo2002@googlemail.com])

# Check for source folder availability
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Basic compiler settings
AC_PROG_CC
AC_PROG_INSTALL
AC_HEADER_STDC

# Automake initialization
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Localization
AH_TEMPLATE([GETTEXT_PACKAGE])
GETTEXT_PACKAGE=gskat
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [GETTEXT package name])
AC_SUBST(GETTEXT_PACKAGE)

AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])
AC_PROG_LIBTOOL

# AC_PROG_INTLTOOL

# Define DATA_DIR variable
if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(DATA_DIR,
        "${ac_default_prefix}/share/pixmaps/${PACKAGE}",
        [Path to card images])
else
    AC_DEFINE_UNQUOTED(DATA_DIR,
        "${prefix}/share/pixmaps/${PACKAGE}",
        [Path to card images])
fi

# Export compiler/linker options
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

# Look for Gtk libs
GTK_VERSION="gtk+-2.0 >= 2.4.0"
PKG_CHECK_MODULES([GTK], $GTK_VERSION)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Compiler flags
# CHECK_CFLAG([-Wall])
# CHECK_CFLAG([-Wextra])

# Generate files
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    data/Makefile
    po/Makefile.in
])

AC_OUTPUT
