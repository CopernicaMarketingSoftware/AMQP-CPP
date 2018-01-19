CPP			= g++
CPPFLAGS		= -Wall -c -I. -O2 -flto -std=c++11 -g
LD			= g++
LDFLAGS			= -lpthread -lboost_system -lamqpcpp
SOURCES			= $(wildcard *.cpp)
OBJECTS			= producer.o consumer.o

all:	producer	\
	consumer


producer: ${OBJECTS}
	${LD} -o $@ producer.o ${LDFLAGS}

consumer: ${OBJECTS}
	${LD} -o $@ consumer.o ${LDFLAGS}


clean:
	${RM} *.obj *~* ${OBJECTS} ${RESULT}

${OBJECTS}:
	${CPP} ${CPPFLAGS} -o $@ ${@:%.o=%.cpp}
