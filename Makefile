PREFIX                  = /usr
INCLUDE_DIR             = ${PREFIX}/include
LIBRARY_DIR             = ${PREFIX}/lib
COPERNICA_INCLUDE_DIR   = ${INCLUDE_DIR}/copernica

all:
		$(MAKE) -C src all

clean:
		$(MAKE) -C src clean

install:
		mkdir -p ${COPERNICA_INCLUDE_DIR}/amqp
		mkdir -p ${LIBRARY_DIR}
		cp -f amqp.h ${COPERNICA_INCLUDE_DIR}
		cp -f include/*.h ${COPERNICA_INCLUDE_DIR}/amqp
		cp -f src/libcopernica_amqp.so ${LIBRARY_DIR}
		cp -f src/libcopernica_amqp.a ${LIBRARY_DIR}
