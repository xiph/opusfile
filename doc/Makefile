## GNU makefile for opusfile documentation.

all: doxygen

doxygen: Doxyfile ../include/opusfile.h
	doxygen

pdf: doxygen
	make -C latex

clean:
	$(RM) -r html
	$(RM) -r latex

distclean: clean
	$(RM) Doxyfile

.PHONY: all clean distclean doxygen pdf

# run autoconf-like replacements to finalize our config
GIT_VERSION := $(shell /bin/sh git-version.sh)
Doxyfile: Doxyfile.in Makefile
	sed -e 's/@PACKAGE@/opusfile/' \
	    -e 's/@VERSION@/$(GIT_VERSION)/' \
	    -e 's/@top_srcdir@/../' \
	  < $< > $@
