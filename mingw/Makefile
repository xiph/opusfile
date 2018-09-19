# Cross-compile opusfile under mingw

TOOL_PREFIX ?= i686-w64-mingw32

# To build opusfile under mingw, we first need to build:
DEPS = ogg opus ssl

ogg_URL := https://downloads.xiph.org/releases/ogg/libogg-1.3.3.tar.xz
ogg_SHA := 4f3fc6178a533d392064f14776b23c397ed4b9f48f5de297aba73b643f955c08

opus_URL := https://archive.mozilla.org/pub/opus/opus-1.3-rc2.tar.gz
opus_SHA := f6bab321fb81db984766f1e4d340a9e71a5ca2c5d4d53f4ee072e84afda271ca

ssl_URL := https://openssl.org/source/openssl-1.0.2p.tar.gz
ssl_SHA := 50a98e07b1a89eb8f6a99477f262df71c6fa7bef77df4dc83025a2845c827d00

all: $(DEPS)

libopusfile-0.dll: ../unix/Makefile $(DEPS)
	CC=$(TOOL_PREFIX)-gcc \
	RANLIB=$(TOOL_PREFIX)-ranlib \
	PKG_CONFIG_PATH=$(CURDIR)/lib/pkgconfig \
	$(MAKE) -f $<

opusfile: $(DEPS)
	../configure --host=$(TOOL_PREFIX) --prefix=$(CURDIR) \
	  PKG_CONFIG_PATH=$(CURDIR)/lib/pkgconfig
	$(MAKE)

clean:
	$(RM) -r objs
	$(RM) -r  bin include lib share ssl
	$(RM) -r $(DEP_DIRS)
	$(RM) opusfile_example.exe seeking_example.exe
	$(RM) libopusfile.a libopusurl.a

# Generate rules to download and verify each dependency.
define WGET_template =
 # Generate tarball name from the url.
 DEP_TARBALLS += $$(notdir $$($(1)_URL))
 $(1)_DIR := $$(basename $$(basename $$(notdir $$($(1)_URL))))
 DEP_DIRS += $$($(1)_DIR)

 # Verify and unpack tarball.
 $$($(1)_DIR): $$(notdir $$($(1)_URL))
	@if test "$$($(1)_SHA)" = "$$$$(sha256sum $$< | cut -f 1 -d ' ')"; \
	then \
	  echo "+ $$< checksum verified."; \
	else \
	  echo "! $$< checksum didn't match!"; \
	  $(RM) $$<; exit 1; \
	fi
	tar xf $$<

 # Fetch tarball from the url.
 $$(notdir $$($(1)_URL)):
	wget $$($(1)_URL)

 # Hook project-specific build rule.
 $(1): $(1)_BUILD
endef
$(foreach dep,$(DEPS),$(eval $(call WGET_template,$(dep))))

fetch: $(DEP_TARBALLS)

realclean: clean
	$(RM) $(DEP_TARBALLS)

# Build scripts for each specific target.

# NOTE: 'make check' generally requires wine with cross-compiling.
ogg_BUILD: $(ogg_DIR)
	cd $< && ./configure --host=$(TOOL_PREFIX) --prefix=$(CURDIR)
	$(MAKE) -C $< install

opus_BUILD: $(opus_DIR)
	cd $< && ./configure --host=$(TOOL_PREFIX) --prefix=$(CURDIR)
	$(MAKE) -C $< install

ssl_BUILD: $(ssl_DIR)
	cd $< && ./Configure shared mingw64 no-asm \
	  --prefix=$(CURDIR) \
	  --cross-compile-prefix=$(TOOL_PREFIX)-
	$(MAKE) -C $< depend
	$(MAKE) -C $<
	$(MAKE) -C $< install

# CROSS_COMPILE="i686-w64-mingw32-" ./Configure mingw no-asm no-shared --prefix=$PWD/mingw && make depend && make -j8 && make install

# Package the binaries.
DIST_VERSION := $(shell git describe --dirty)
DIST := opusfile-$(DIST_VERSION)-win32
package: $(DIST).zip

$(DIST).zip: $(DIST)
	zip -r $@ $</*
	@echo $@ ready to go.

$(DIST): $(addprefix $(CURDIR)/bin/, libogg-0.dll libopus-0.dll ssleay32.dll) 
	cd .. && make install
	mkdir -p $(DIST)
	cp ../AUTHORS ../COPYING ../README.md ../include/opusfile.h $@
	cp ../.libs/libopusfile-0.dll $@
	cp ../.libs/libopusfile.a $@
	cp ../.libs/libopusfile.dll.a $@
	cp ../.libs/libopusurl-0.dll $@
	cp ../.libs/libopusurl.a $@
	cp ../.libs/libopusurl.dll.a $@
	cp bin/*.dll $@
	cp ../examples/.libs/*.exe $@
	cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_sjlj-1.dll $@
	cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libwinpthread-1.dll $@
	i686-w64-mingw32-strip $@/*.exe
	i686-w64-mingw32-strip $@/*.dll
	i686-w64-mingw32-strip $@/*.a
	cd $@ && sha256sum * > SHA256SUMS.txt
