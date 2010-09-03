#!/bin/bash

# Create a ChangeLog file if necessary
if test -d .git; then
    test -e ChangeLog ||
        git log --pretty=medium | fold -s > ChangeLog
fi

aclocal -I m4

libtoolize --automake
intltoolize --automake

autoheader
automake --add-missing --copy
autoconf
