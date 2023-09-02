# Adapted to test caio's Z80 implementation
# The original file is Makefile.orig.

ROOT=		${abspath ../../../../../}

include ${ROOT}/mk/config.mk

SJASM=		${TOOLS_BINDIR}/sjasm
NAME=		z80test
VERSION=	1.2
PKG=		$(NAME)-$(VERSION)

PROGS=		z80full \
		z80flags \
		z80doc \
		z80docflags \
		z80ccf \
		z80memptr \
		z80ccfscr

SRCS=		${PROGS:%=%.asm}

OBJS=		${SRCS:%.asm=%.out}

CLEANFILES=	*.out *.lst ${PROGS} ${OBJS}

all: ${SJASM} ${OBJS}

debug: all

%.out: %.asm
	${SJASM} $<

${SJASM}:
	${MAKE} ${MARKEARGS} -C${TOOLSDIR} all

include ${ROOT}/mk/clean.mk
