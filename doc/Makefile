## GNU makefile for opusfile documentation.

all: doxygen

doxygen: Doxyfile ../include/opusfile.h
	doxygen

pdf: doxygen
	make -C latex

clean:
	$(RM) -r html
	$(RM) -r latex

.PHONY: all clean doxygen
