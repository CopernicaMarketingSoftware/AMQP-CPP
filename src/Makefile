CPP	        		= g++
RM	        		= rm -f
CPPFLAGS			= -Wall -c -I../include -std=c++11 -MD -Wno-class-conversion
LD	        		= g++
LD_FLAGS			= -Wall -shared
SHARED_LIB			= lib$(LIBRARY_NAME).so.$(VERSION)
STATIC_LIB			= lib$(LIBRARY_NAME).a.$(VERSION)
SOURCES				= $(wildcard *.cpp) $(wildcard linux_tcp/*.cpp)
SHARED_OBJECTS		= $(SOURCES:%.cpp=%.o)
STATIC_OBJECTS		= $(SOURCES:%.cpp=%.s.o)
DEPENDENCIES		= $(SOURCES:%.cpp=%.d)
PURE_SHARED_OBJECTS	= $(filter-out tcpconnection.o, $(SOURCES:%.cpp=%.o))
PURE_STATIC_OBJECTS	= $(filter-out tcpconnection.s.o, $(SOURCES:%.cpp=%.s.o))


ifeq ($(shell uname -s),Darwin)
    SONAMEPARAMETER = -install_name
else
    SONAMEPARAMETER = -soname
endif

-include ${DEPENDENCIES}

all:			CPPFLAGS += -g
all:			LD_FLAGS += -g
all:			shared static

pure:			CPPFLAGS += -g
pure:			LD_FLAGS += -g
pure:			shared_pure static_pure

release:		CPPFLAGS += -O2
release:		LD_FLAGS += -O2
release:		shared static

shared:			${SHARED_OBJECTS} ${SHARED_LIB}
shared_pure:	${PURE_SHARED_OBJECTS} ${SHARED_LIB}

static:			${STATIC_OBJECTS} ${STATIC_LIB}
static_pure:	${PURE_STATIC_OBJECTS} ${STATIC_LIB}

${SHARED_LIB}: ${SHARED_OBJECTS}
	${LD} ${LD_FLAGS} -Wl,${SONAMEPARAMETER},lib$(LIBRARY_NAME).so.$(SONAME) -o $@ ${SHARED_OBJECTS}

${STATIC_LIB}: ${STATIC_OBJECTS}
	ar rcs ${STATIC_LIB} ${STATIC_OBJECTS}

clean:
	${RM} *.obj *~* ${SHARED_OBJECTS} ${STATIC_OBJECTS} ${SHARED_LIB} ${STATIC_LIB} ${DEPENDENCIES}

${SHARED_OBJECTS}:
	${CPP} ${CPPFLAGS} -fpic -o $@ ${@:%.o=%.cpp}

${STATIC_OBJECTS}:
	${CPP} ${CPPFLAGS} -o $@ ${@:%.s.o=%.cpp}
