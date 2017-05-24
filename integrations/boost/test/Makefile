CPP			    = g++
CPPFLAGS		= -Wall -c -I. -O2 -flto -std=c++11 -g
LD			    = g++
LDFLAGS			= -lamqpcpp -lboost_system -pthread
#use these flags for ssl support
#LDFLAGS			= -lamqpcpp -lboost_system -pthread -lssl -lcrypto -ldl

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

