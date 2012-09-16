# NOTE: This Makefile requires GNU make
# Location to put the targets.
TARGETBINDIR = .
TESTBINDIR = tests
TARGETLIBDIR = .
# Name of the targets
LIBOPUSFILE_TARGET = libopusfile.a
OPUSFILE_EXAMPLE_TARGET = opusfile_example
SEEKING_EXAMPLE_TARGET = seeking_example
# Test targets
#TODO: tests
FOO_TARGET = foo
# The command to use to generate dependency information
MAKEDEPEND = ${CC} -MM
#MAKEDEPEND = makedepend -f- -Y --
# The location of include files.
# Modify these to point to your Ogg and Vorbis include directories if they are
#  not installed in a standard location.
CINCLUDE = `pkg-config --cflags ogg opus`
# These are gcc-only, but not actually critical.
# Extra compilation flags.
# You may get speed increases by including flags such as -O2 or -O3 or
#  -ffast-math, or additional flags, depending on your system and compiler.
# The -g flag will generally include debugging information.
CFLAGS := -g $(CFLAGS)
CFLAGS := -DOP_ENABLE_ASSERTIONS $(CFLAGS)
CFLAGS := -fPIC $(CFLAGS)
CFLAGS := -std=c89 -pedantic $(CFLAGS)
CFLAGS := -fvisibility=hidden $(CFLAGS)
CFLAGS := -Wextra -Wno-parentheses -Wno-long-long $(CFLAGS)
CFLAGS := -Wall $(CFLAGS)

# Libraries to link with, and the location of library files.
LIBS = `pkg-config --libs ogg opus`

#TODO: tests
SEEKING_EXAMPLE_LIBS = -lm

# ANYTHING BELOW THIS LINE PROBABLY DOES NOT NEED EDITING
CINCLUDE := -I../include ${CINCLUDE}
LIBSRCDIR = ../src
BINSRCDIR = ../examples
TESTSRCDIR = ${LIBSRCDIR}
WORKDIR = objs

# C source file lists
LIBOPUSFILE_CSOURCES = \
info.c \
internal.c \
opusfile.c \
stream.c \

LIBOPUSFILE_CHEADERS = \
internal.h \

OPUSFILE_EXAMPLE_CSOURCES = opusfile_example.c

SEEKING_EXAMPLE_CSOURCES = seeking_example.c

FOO_CSOURCES = tests/foo.c

# Create object file list.
LIBOPUSFILE_OBJS:= ${LIBOPUSFILE_CSOURCES:%.c=${WORKDIR}/%.o}
LIBOPUSFILE_ASMS:= ${LIBOPUSFILE_OBJS:%.o=%.s}
LIBOPUSFILE_DEPS:= ${LIBOPUSFILE_OBJS:%.o=%.d}
OPUSFILE_EXAMPLE_OBJS:= ${OPUSFILE_EXAMPLE_CSOURCES:%.c=${WORKDIR}/%.o}
SEEKING_EXAMPLE_OBJS:= ${SEEKING_EXAMPLE_CSOURCES:%.c=${WORKDIR}/%.o}
#TODO: tests
FOO_OBJS:= ${FOO_CSOURCES:%.c=${WORKDIR}/%.o}
ALL_OBJS:= \
 ${LIBOPUSFILE_OBJS} \
 ${OPUSFILE_EXAMPLE_OBJS} \
 ${SEEKING_EXAMPLE_OBJS} \

#TODO: tests
# ${FOO_OBJS}

# Create the dependency file list
ALL_DEPS:= ${ALL_OBJS:%.o=%.d}
# Prepend source path to file names.
LIBOPUSFILE_CSOURCES:= ${LIBOPUSFILE_CSOURCES:%=${LIBSRCDIR}/%}
LIBOPUSFILE_CHEADERS:= ${LIBOPUSFILE_CHEADERS:%=${LIBSRCDIR}/%}
OPUSFILE_EXAMPLE_CSOURCES:= ${OPUSFILE_EXAMPLE_CSOURCES:%=${BINSRCDIR}/%}
SEEKING_EXAMPLE_CSOURCES:= ${SEEKING_EXAMPLE_CSOURCES:%=${BINSRCDIR}/%}
#TODO: tests
FOO_CSOURCES:= ${FOO_CSOURCES:%=${TESTSRCDIR}/%}
ALL_CSOURCES:= \
 ${LIBOPUSFILE_CSOURCES} \
 ${OPUSFILE_EXAMPLE_CSOURCES} \
 ${SEEKING_EXAMPLE_CSOURCES} \

#TODO: tests
# ${FOO_CSOURCES} \
# Prepand target path to file names.
LIBOPUSFILE_TARGET:= ${TARGETLIBDIR}/${LIBOPUSFILE_TARGET}
OPUSFILE_EXAMPLE_TARGET:= ${TARGETBINDIR}/${OPUSFILE_EXAMPLE_TARGET}
SEEKING_EXAMPLE_TARGET:= ${TARGETBINDIR}/${SEEKING_EXAMPLE_TARGET}
# Prepend test path to file names.
#TODO: tests
FOO_TARGET:= ${TESTBINDIR}/${FOO_TARGET}
# Complete set of targets
ALL_TARGETS:= \
 ${LIBOPUSFILE_TARGET} \
 ${OPUSFILE_EXAMPLE_TARGET} \
 ${SEEKING_EXAMPLE_TARGET} \

#TODO: tests
# ${FOO_TARGET} \

# Targets:
# Everything (default)
all: ${ALL_TARGETS}

# libopusfile
${LIBOPUSFILE_TARGET}: ${LIBOPUSFILE_OBJS}
	mkdir -p ${TARGETLIBDIR}
	ar cqs $@ ${LIBOPUSFILE_OBJS}

# opusfile_example
${OPUSFILE_EXAMPLE_TARGET}: ${OPUSFILE_EXAMPLE_OBJS} ${LIBOPUSFILE_TARGET}
	mkdir -p ${TARGETBINDIR}
	${CC} ${CFLAGS} ${OPUSFILE_EXAMPLE_OBJS} ${LIBS} \
         ${SEEKING_EXAMPLE_LIBS} ${LIBOPUSFILE_TARGET} -o $@

# seeking_example
${SEEKING_EXAMPLE_TARGET}: ${SEEKING_EXAMPLE_OBJS} ${LIBOPUSFILE_TARGET}
	mkdir -p ${TARGETBINDIR}
	${CC} ${CFLAGS} ${SEEKING_EXAMPLE_OBJS} ${LIBS} ${LIBOPUSFILE_TARGET} \
         -o $@

#TODO: tests
# foo
#${FOO_TARGET}: ${FOO_OBJS}
#	mkdir -p ${TESTBINDIR}
#	${CC} ${CFLAGS} ${FOO_OBJS} ${FOO_LIBS} -o $@

# Assembly listing
ALL_ASM := ${ALL_OBJS:%.o=%.s}
asm: ${ALL_ASM}

# Check that build is complete.
check: all
#TODO: tests
#	${FOO_TARGET}

# Remove all targets.
clean:
	${RM} ${ALL_ASM} ${ALL_OBJS} ${ALL_DEPS}
	${RM} ${ALL_TARGETS}
	-rmdir ${TESTBINDIR} ${WORKDIR}/tests ${WORKDIR}/x86 ${WORKDIR}

# Make everything depend on changes in the Makefile
${ALL_ASM} ${ALL_OBJS} ${ALL_DEPS} ${ALL_TARGETS} : Makefile

# Specify which targets are phony for GNU make
.PHONY : all clean check

# Rules
${WORKDIR}/%.d: ${LIBSRCDIR}/%.c
	mkdir -p ${dir $@}
	${MAKEDEPEND} ${CINCLUDE} ${CFLAGS} $< -MT ${@:%.d=%.o} > $@
	${MAKEDEPEND} ${CINCLUDE} ${CFLAGS} $< -MT ${@:%.d=%.s} >> $@
	${MAKEDEPEND} ${CINCLUDE} ${CFLAGS} $< -MT $@ >> $@
${WORKDIR}/%.s: ${LIBSRCDIR}/%.c
	mkdir -p ${dir $@}
	${CC} ${CINCLUDE} ${CFLAGS} -S -o $@ $<
${WORKDIR}/%.o: ${LIBSRCDIR}/%.c
	mkdir -p ${dir $@}
	${CC} ${CINCLUDE} ${CFLAGS} -c -o $@ $<

${WORKDIR}/%.d : ${BINSRCDIR}/%.c
	mkdir -p ${dir $@}
	${MAKEDEPEND} ${CINCLUDE} ${CFLAGS} $< -MT ${@:%.d=%.o} > $@
${WORKDIR}/%.s : ${BINSRCDIR}/%.c ${WORKDIR}/%.o
	mkdir -p ${dir $@}
	${CC} ${CINCLUDE} ${CFLAGS} -S -o $@ $<
${WORKDIR}/%.o : ${BINSRCDIR}/%.c
	mkdir -p ${dir $@}
	${CC} ${CINCLUDE} ${CFLAGS} -c -o $@ $<

# Include header file dependencies
include ${ALL_DEPS}
