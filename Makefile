CC			= gcc
CFLAGS			= -c -fPIC -Wall -Wextra -Wfatal-errors -Wno-unused-function
LDFLAGS      = -shared
SRCDIR 			= src
DEBUGOUTPUTDIR 		= build/debug
RELEASEOUTPUTDIR	= build/release
SOURCES			= src/cd_wq.c src/cd_log.c
INCLUDES		= -I./src -Iinclude
_OBJECTS		= $(SOURCES:.c=.o)
DEBUGOBJECTS 		= $(patsubst src/%,$(DEBUGOUTPUTDIR)/%,$(_OBJECTS))
RELEASEOBJECTS 		= $(patsubst src/%,$(RELEASEOUTPUTDIR)/%,$(_OBJECTS))
DEBUGTARGET		= build/debug/libcd.so
RELEASETARGET	= build/release/libcd.so

debugprereqs:
		mkdir -p $(DEBUGOUTPUTDIR)

releaseprereqs:
		mkdir -p $(RELEASEOUTPUTDIR)

install-prereqs:
		sudo mkdir -p /usr/local/include/cd

install-post:
	$(eval ldconf := $(shell sudo which ldconfig))
	$(if $(ldconf), $(shell sudo ldconfig), $(shell echo "Warning: no ldconfig on this system (make sure your linker is updated with libcd)"))

debugall:	debugprereqs $(DEBUGOBJECTS) $(DEBUGTARGET)
releaseall:	releaseprereqs $(RELEASETARGET)

# additional flags
# CONFIG_DEBUG_LIST	- extensive debugging of list with external debugging
# 			functions
debug:		CFLAGS += -g -ggdb3 -O0
debug:		debugall

release:	CFLAGS +=
release: 	releaseall

test-debug:		debugall
		cd test && make test-debug

test-release:		releaseall
		cd test && make test-release

test:		test-release

test-clean:
		cd test && make clean

examples-debug:		debugall install-debug
		cd examples && make examples-debug

examples-release:		releaseall install-release
		cd examples && make examples-release

examples:		examples-release

examples-clean:
		cd examples && make clean

$(DEBUGTARGET): $(DEBUGOBJECTS) 
	$(CC) $(DEBUGOBJECTS) -o $@ $(LDFLAGS)

$(RELEASETARGET): $(RELEASEOBJECTS) 
	$(CC) $(RELEASEOBJECTS) -o $@ $(LDFLAGS)

$(DEBUGOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(RELEASEOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

all: release

.DEFAULT_GOAL = release

install-headers: install-prereqs include/cd.h
	sudo cp include/* /usr/local/include/cd/

install-debug: $(DEBUGTARGET) install-headers
	sudo cp $(DEBUGTARGET) /lib/
	make install-post

install-release: $(RELEASETARGET) install-headers
	sudo cp $(RELEASETARGET) /lib/
	make install-post

install: install-release

uninstall:
	sudo rm /lib/libcd.so
	sudo rm -rf /usr/local/include/cd
	make install-post

clean:
	rm -rf $(DEBUGOBJECTS) $(DEBUGTARGET) $(RELEASEOBJECTS) $(RELEASETARGET)

clean-all: clean test-clean examples-clean
