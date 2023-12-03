# Adapted to test caio's Z80 implementation
# The original file is Makefile.orig.

ROOT=		${abspath ../../../../../}

include ${ROOT}/mk/config.mk

SJASM=		${TOOLS_BINDIR}/sjasm
NAME=		z80test

SRCS=		z80full.asm \
		z80flags.asm \
		z80doc.asm \
		z80docflags.asm \
		z80ccf.asm \
		z80memptr.asm \
		z80ccfscr.asm

PROGS=		${SRCS:%.asm=%.out}

CLEANFILES=	*.lst

DISTCLEANFILES=	${PROGS}

all: ${SJASM} ${PROGS}

debug: all

%.out: %.asm
	${SJASM} $<

${SJASM}:
	${MAKE} ${MARKEARGS} -C${TOOLSDIR} all

include ${ROOT}/mk/clean.mk
