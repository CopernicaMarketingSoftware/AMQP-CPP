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
		mkdir -p ${INCLUDE_DIR}/libamqp
		mkdir -p ${LIBRARY_DIR}
		cp -f libamqp.h ${INCLUDE_DIR}
		cp -f include/*.h ${INCLUDE_DIR}/libamqp
		cp -f src/liblibamqp.so ${LIBRARY_DIR}
		cp -f src/liblibamqp.a ${LIBRARY_DIR}
