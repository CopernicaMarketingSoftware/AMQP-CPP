PREFIX                  = /usr
INCLUDE_DIR             = ${PREFIX}/include
LIBRARY_DIR             = ${PREFIX}/lib

all:
		$(MAKE) -C src all

clean:
		$(MAKE) -C src clean

install:
		mkdir -p ${INCLUDE_DIR}/libamqp
		mkdir -p ${LIBRARY_DIR}
		cp -f libamqp.h ${INCLUDE_DIR}
		cp -f include/*.h ${INCLUDE_DIR}/amqp
		cp -f src/libamqp.so ${LIBRARY_DIR}
		cp -f src/libamqp.a ${LIBRARY_DIR}
