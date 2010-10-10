#!/bin/bash

# Create a ChangeLog file if necessary
if test -d .git; then
    test -e ChangeLog ||
        git log --date-order --date=short |\
            sed -e '/^commit.*$/d' |\
            awk '/^Author/ {sub(/\\$/, ""); getline t; print $0 t; next}; 1' |\
            sed -e 's/^Author: //g' |\
            sed -e 's/>Date:   \([0-9]*-[0-9]*-[0-9]*\)/>\t\1/g' |\
            sed -e 's/^[ \t]\+/\t* /g' > ChangeLog
fi

aclocal

libtoolize --automake
intltoolize --automake

# gtkdocize || exit 1

autoheader
automake --add-missing --foreign --copy
autoconf

./configure $@
