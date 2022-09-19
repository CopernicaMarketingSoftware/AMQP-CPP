PREFIX                  ?= /usr
INCLUDE_DIR             = ${PREFIX}/include
LIBRARY_DIR             = ${PREFIX}/lib
export LIBRARY_NAME		= amqpcpp
export SONAME			= 4.3
export VERSION			= 4.3.18

all:
		$(MAKE) -C src all

pure:
		$(MAKE) -C src pure

release:
		$(MAKE) -C src release

static:
		$(MAKE) -C src static

shared:
		$(MAKE) -C src shared

clean:
		$(MAKE) -C src clean

install:
		mkdir -p ${INCLUDE_DIR}/$(LIBRARY_NAME)
		mkdir -p ${INCLUDE_DIR}/$(LIBRARY_NAME)/linux_tcp
		mkdir -p ${LIBRARY_DIR}
		cp -f include/$(LIBRARY_NAME).h ${INCLUDE_DIR}
		cp -f include/amqpcpp/*.h ${INCLUDE_DIR}/$(LIBRARY_NAME)
		cp -f include/amqpcpp/linux_tcp/*.h ${INCLUDE_DIR}/$(LIBRARY_NAME)/linux_tcp
		-cp -f src/lib$(LIBRARY_NAME).so.$(VERSION) ${LIBRARY_DIR}
		-cp -f src/lib$(LIBRARY_NAME).a.$(VERSION) ${LIBRARY_DIR}
		ln -s -f lib$(LIBRARY_NAME).so.$(VERSION) $(LIBRARY_DIR)/lib$(LIBRARY_NAME).so.$(SONAME)
		ln -s -f lib$(LIBRARY_NAME).so.$(VERSION) $(LIBRARY_DIR)/lib$(LIBRARY_NAME).so
		ln -s -f lib$(LIBRARY_NAME).a.$(VERSION) $(LIBRARY_DIR)/lib$(LIBRARY_NAME).a
