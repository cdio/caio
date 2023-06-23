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

CXXFLAGS+=	-O2

LDFLAGS?=

BIN=		sjasm

all: ${BIN}

${BIN}: ${OBJS}
	${CXX} ${LDFLAGS} -o $@ $^

%.o: %.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c -o $@ $<

clean:
	rm -f ${BIN} ${OBJS}
