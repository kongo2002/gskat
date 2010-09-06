AC_PREREQ([2.59])

# Set gskat version
m4_define(gskat_major_version, 0)
m4_define(gskat_minor_version, 0)
m4_define(gskat_micro_version, 1)
m4_define(gskat_version, gskat_major_version.gskat_minor_version.gskat_micro_version)

# Initial project information
AC_INIT([gskat], gskat_version, [kongo2002@googlemail.com])

# Export version
AC_DEFINE(GSKAT_MAJOR_VERSION, gskat_major_version, [gskat major version])
AC_SUBST(GSKAT_MAJOR_VERSION, gskat_major_version)
AC_DEFINE(GSKAT_MINOR_VERSION, gskat_minor_version, [gskat minor version])
AC_SUBST(GSKAT_MINOR_VERSION, gskat_minor_version)
AC_DEFINE(GSKAT_MICRO_VERSION, gskat_micro_version, [gskat micro version])
AC_SUBST(GSKAT_MICRO_VERSION, gskat_micro_version)

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

# Try to define git commit hash
commit_hash="gskat_version"
if test -d ".git" -o -f ".git"; then
    commit_hash="`git describe --always`"
fi

AC_DEFINE_UNQUOTED(GSKAT_COMMIT_HASH,
    "${commit_hash}",
    [Git commit hash])

# Export compiler/linker options
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

# Look for Gtk libs
GTK_VERSION="gtk+-2.0 >= 2.4.0"
PKG_CHECK_MODULES([GTK], $GTK_VERSION)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Generate files
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    data/Makefile
    data/icons/Makefile
    po/Makefile.in
])

AC_OUTPUT
