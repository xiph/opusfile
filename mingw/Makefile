# Cross-compile opusfile under mingw

TOOL_PREFIX ?= i686-w64-mingw32

# To build opusfile under mingw, we first need to build:
DEPS = ogg opus ssl

ogg_URL := http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.xz
ogg_SHA := 3f687ccdd5ac8b52d76328fbbfebc70c459a40ea891dbf3dccb74a210826e79b

opus_URL := https://archive.mozilla.org/pub/opus/opus-1.1.1.tar.gz
opus_SHA := 9b84ff56bd7720d5554103c557664efac2b8b18acc4bbcc234cb881ab9a3371e

ssl_URL := https://openssl.org/source/openssl-1.0.1q.tar.gz
ssl_SHA := b3658b84e9ea606a5ded3c972a5517cd785282e7ea86b20c78aa4b773a047fb7

libopusfile-0.dll: ../unix/Makefile $(DEPS)
	CC=$(TOOL_PREFIX)-gcc \
	RANLIB=$(TOOL_PREFIX)-ranlib \
	PKG_CONFIG_PATH=${PWD}/lib/pkgconfig \
	$(MAKE) -f $<

opusfile: $(DEPS)
	../configure --host=$(TOOL_PREFIX) --prefix=${PWD} \
	  PKG_CONFIG_PATH=${PWD}/lib/pkgconfig
	$(MAKE)

clean:
	$(RM) -r objs

# Generate rules to download and verify each dependency.
define WGET_template =
 # Generate unpacked tarball name from the url.
 $(1)_DIR := $$(basename $$(basename $$(notdir $$($(1)_URL))))

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

# Build scripts for each specific target.

# NOTE: 'make check' generally requires wine with cross-compiling.
ogg_BUILD: $(ogg_DIR)
	cd $< && ./configure --host=$(TOOL_PREFIX) --prefix=${PWD}
	$(MAKE) -C $< check
	$(MAKE) -C $< install

opus_BUILD: $(opus_DIR)
	cd $< && ./configure --host=$(TOOL_PREFIX) --prefix=${PWD}
	$(MAKE) -C $< check
	$(MAKE) -C $< install

ssl_BUILD: $(ssl_DIR)
	cd $< && ./Configure shared mingw64 no-asm \
	  --prefix=${PWD} \
	  --cross-compile-prefix=$(TOOL_PREFIX)-
	$(MAKE) -C $< depend
	$(MAKE) -C $<
	$(MAKE) -C $< install

# CROSS_COMPILE="i686-w64-mingw32-" ./Configure mingw no-asm no-shared --prefix=$PWD/mingw && make depend && make -j8 && make install
