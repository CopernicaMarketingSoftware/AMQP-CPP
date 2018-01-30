CPP				= g++
CPPFLAGS		= -Wall -c -I. -O2 -flto -std=c++11 -g
LD				= g++
LDFLAGS			= -lpthread -lboost_system -lamqpcpp
SOURCES			= $(wildcard *.cpOBJECTS
OBJECTS			= new_task.o worker.o
all:			\
	new_task	\
	worker


new_task: ${OBJECTS}
	${LD} -o $@ new_task.o ${LDFLAGS}

worker: ${OBJECTS}
	${LD} -o $@ worker.o ${LDFLAGS}


clean:
	${RM} *.obj *~* ${OBJECTS} ${RESULT}

${OBJECTS}:
	${CPP} ${CPPFLAGS} -o $@ ${@:%.o=%.cpp}
