PREFIX                  ?= /usr
INCLUDE_DIR             = ${PREFIX}/include
LIBRARY_DIR             = ${PREFIX}/lib
export LIBRARY_NAME		= amqpcpp
export SONAME			= 2.6
export VERSION			= 2.6.2

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
		mkdir -p ${LIBRARY_DIR}
		cp -f $(LIBRARY_NAME).h ${INCLUDE_DIR}
		cp -f include/*.h ${INCLUDE_DIR}/$(LIBRARY_NAME)
		-cp -f src/lib$(LIBRARY_NAME).so.$(VERSION) ${LIBRARY_DIR}
		-cp -f src/lib$(LIBRARY_NAME).a.$(VERSION) ${LIBRARY_DIR}
		ln -r -s -f $(LIBRARY_DIR)/lib$(LIBRARY_NAME).so.$(VERSION) $(LIBRARY_DIR)/lib$(LIBRARY_NAME).so.$(SONAME)
		ln -r -s -f $(LIBRARY_DIR)/lib$(LIBRARY_NAME).so.$(VERSION) $(LIBRARY_DIR)/lib$(LIBRARY_NAME).so
		ln -r -s -f $(LIBRARY_DIR)/lib$(LIBRARY_NAME).a.$(VERSION) $(LIBRARY_DIR)/lib$(LIBRARY_NAME).a
