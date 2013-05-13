ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -I$(top_srcdir)/include $(DEPS_CFLAGS)

dist_doc_DATA = COPYING AUTHORS README.txt

opusincludedir = ${includedir}/opus
opusinclude_HEADERS = include/opusfile.h

lib_LTLIBRARIES = libopusfile.la
libopusfile_la_SOURCES = \
	src/info.c \
	src/internal.c src/internal.h \
	src/opusfile.c src/stream.c
libopusfile_la_LIBADD = $(DEPS_LIBS)
libopusfile_la_LDFLAGS = -no-undefined \
 -version-info @OP_LT_CURRENT@:@OP_LT_REVISION@:@OP_LT_AGE@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = opusfile.pc

if OP_ENABLE_HTTP
lib_LTLIBRARIES += libopusurl.la
libopusurl_la_SOURCES = src/http.c
libopusurl_la_LIBADD = libopusfile.la $(URL_DEPS_LIBS)
libopusurl_la_LDFLAGS = -no-undefined \
 -version-info @OP_LT_CURRENT@:@OP_LT_REVISION@:@OP_LT_AGE@

pkgconfig_DATA += opusurl.pc

examples_opusfile_example_LDADD = libopusurl.la libopusfile.la
examples_seeking_example_LDADD = libopusurl.la libopusfile.la

if OP_WIN32
libopusurl_la_SOURCES += src/wincerts.c
libopusurl_la_LIBADD += -lws2_32 -lcrypt32
endif

else
examples_opusfile_example_LDADD = libopusfile.la
examples_seeking_example_LDADD = libopusfile.la
endif

noinst_PROGRAMS = examples/opusfile_example examples/seeking_example

debug:
	$(MAKE) CFLAGS="${CFLAGS} -O0 -ggdb -DOP_ENABLE_ASSERTIONS" all

EXTRA_DIST = \
 opusfile.pc.in \
 opusurl.pc.in \
 opusfile-uninstalled.pc.in \
 opusurl-uninstalled.pc.in \
 doc/Doxyfile.in \
 doc/git-version.sh \
 doc/opus_logo.svg \
 doc/Makefile \
 unix/Makefile

# Targets to build and install just the library without the docs
opusfile install-opusfile: NO_DOXYGEN = 1

opusfile: all
install-opusfile: install

# Or just the docs
docs: doc/doxygen-build.stamp

install-docs:
	@if [ -z "$(NO_DOXYGEN)" ]; then \
	  ( cd doc && \
	    echo "Installing documentation in $(DESTDIR)$(docdir)"; \
	    for f in `find html -type f \! -name "installdox"` ; do \
	      $(INSTALL) -d $(DESTDIR)$(docdir)/html/search;   \
	      $(INSTALL_DATA) $$f $(DESTDIR)$(docdir)/$$f;     \
	    done ) \
	fi

doc/doxygen-build.stamp: doc/Doxyfile $(top_srcdir)/doc/opus_logo.svg \
                         $(top_srcdir)/include/*.h
	@[ -n "$(NO_DOXYGEN)" ] || ( cd doc && doxygen && touch $(@F) )


if HAVE_DOXYGEN

# Or everything (by default)
all-local: docs

install-data-local: install-docs

clean-local:
	$(RM) -r doc/html
	$(RM) -r doc/latex
	$(RM) doc/doxygen-build.stamp

uninstall-local:
	$(RM) -r $(DESTDIR)$(docdir)/html

endif

.PHONY: opusfile install-opusfile docs install-docs
