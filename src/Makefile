CPP			= g++-4.8
RM			= rm -f
CPPFLAGS		= -Wall -c -I. -O2 -flto -std=c++11 -g
LD			= g++
LD_FLAGS		= -Wall -shared -O2
RESULT			= libcopernica_amqp.so

STATIC			= $(RESULT:%.so=%.a)
SOURCES			= $(wildcard *.cpp)
OBJECTS			= $(SOURCES:%.cpp=%.o)
OBJECTS_STATIC		= $(SOURCES:%.cpp=%.s.o)

all:	${OBJECTS} ${OBJECTS_STATIC} ${RESULT}
	$(MAKE) STATIC_LIB

${RESULT}: ${OBJECTS}
	${LD} ${LD_FLAGS} -o $@ ${OBJECTS}

clean:
	${RM} *.obj *~* ${OBJECTS} ${OBJECTS_STATIC} ${RESULT} ${STATIC}

${OBJECTS}:
	${CPP} ${CPPFLAGS} -fpic -o $@ ${@:%.o=%.cpp}

${OBJECTS_STATIC}:
	${CPP} ${CPPFLAGS} -o $@ ${@:%.s.o=%.cpp}

STATIC_LIB:
	ar rcs ${STATIC} ${OBJECTS_STATIC}
