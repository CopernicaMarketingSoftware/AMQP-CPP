CPP				= g++
CPPFLAGS		= -Wall -I. -O3 -flto -std=c++17 -ggdb3
LDFLAGS			= -lamqpcpp -lcopernica_event -lcopernica_network -lev -lpthread -ldl -lssl -lcrypto
SOURCES			= $(wildcard *.cpp)
PROGRAMS		= $(SOURCES:%.cpp=%)

all:	${PROGRAMS}

clean:
	${RM} *~* ${PROGRAMS}

%: %.cpp
	${CPP} $< ${CPPFLAGS} ${LDFLAGS} -o $@
