CPP			= g++
CPPFLAGS		= -Wall -c -I. -O0 -flto -std=c++17 -ggdb3
LD			= g++
LDFLAGS			= -lamqpcpp -lcopernica_event -lcopernica_network -lev -lpthread -ldl
RESULT			= a.out
SOURCES			= $(wildcard *.cpp)
OBJECTS			= $(SOURCES:%.cpp=%.o)

all:	${OBJECTS} ${RESULT}

${RESULT}: ${OBJECTS}
	${LD} -o $@ ${OBJECTS} ${LDFLAGS}

clean:
	${RM} *.obj *~* ${OBJECTS} ${RESULT}

${OBJECTS}:
	${CPP} ${CPPFLAGS} -o $@ ${@:%.o=%.cpp}
