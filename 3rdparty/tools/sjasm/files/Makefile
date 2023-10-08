#
# sjasm uses cmake, caio does not like that thing.
#
CXXSRCS=	datadir.cpp \
		datastructures.cpp \
		directives.cpp \
		errors.cpp \
		expressions.cpp \
		fileio.cpp \
		output.cpp \
		pimsx.cpp \
		preprocessor.cpp \
		rawsource.cpp \
		reader.cpp \
		sjasm.cpp \
		source.cpp

OBJS=		${CXXSRCS:%.cpp=%.o}

CPPFLAGS+=	-I.

CXXFLAGS+=	-std=c++14 \
		-O2 \
		-Wall \
		-Werror

# No comment.
CXXFLAGS+=	-Wno-dangling-else \
		-Wno-parentheses \
		-Wno-switch-bool \
		-Wno-switch \
		-Wno-tautological-constant-out-of-range-compare \
		-Wno-reorder-ctor \
		-Wno-missing-braces \
		-Wno-char-subscripts \
		-Wno-unused-private-field

LDFLAGS?=

BIN=		sjasm

override CXX=	clang++

all: ${BIN}

${BIN}: ${OBJS}
	${CXX} ${LDFLAGS} -o $@ $^

%.o: %.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c -o $@ $<

clean:
	rm -f ${BIN} ${OBJS}
