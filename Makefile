PREFIX                  = /usr
INCLUDE_DIR             = ${PREFIX}/include
LIBRARY_DIR             = ${PREFIX}/lib

all:
		$(MAKE) -C src all

static:
		$(MAKE) -C src static

shared:
		$(MAKE) -C src shared

clean:
		$(MAKE) -C src clean

install:
		mkdir -p ${INCLUDE_DIR}/amqpcpp
		mkdir -p ${LIBRARY_DIR}
		cp -f amqpcpp.h ${INCLUDE_DIR}
		cp -f include/*.h ${INCLUDE_DIR}/amqpcpp
		cp -f src/libamqpcpp.so ${LIBRARY_DIR}
		cp -f src/libamqpcpp.a ${LIBRARY_DIR}
