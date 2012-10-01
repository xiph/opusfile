#!/bin/sh
# Run this to set up the build system: configure, makefiles, etc.
set -e

package="opusfile"

ACLOCAL_FLAGS="-I m4"

olddir=`pwd`
srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

cd "$srcdir"

echo "Updating build configuration files for $package, please wait...."
autoreconf -isf
