CC			= gcc
CFLAGS			= -c -fPIC -Wall -Wextra -Wfatal-errors -Wno-unused-function
LDFLAGS      = -shared
SRCDIR 			= .
DEBUGOUTPUTDIR 		= build/debug
RELEASEOUTPUTDIR	= build/release
SOURCES			= src/cd_wq.c src/cd_log.c
INCLUDES		= -I./src -Iinclude
_OBJECTS		= $(SOURCES:.c=.o)
DEBUGOBJECTS 		= $(patsubst %,$(DEBUGOUTPUTDIR)/%,$(_OBJECTS))
RELEASEOBJECTS 		= $(patsubst %,$(RELEASEOUTPUTDIR)/%,$(_OBJECTS))
DEBUGTARGET		= build/debug/libcd.so
RELEASETARGET	= build/release/libcd.so

debugall:	$(SOURCES) $(DEBUGTARGET)
releaseall:	$(SOURCES) $(RELEASETARGET)

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

install-debug: $(DEBUGTARGET)
	cp $(DEBUGTARGET) /usr/local/lib/

install-release: $(RELEASETARGET)
	cp $(RELEASETARGET) /usr/local/lib/

install: install-release

uninstall:
	rm /usr/local/lib/libcd.so

clean:
	rm -rf $(DEBUGOBJECTS) $(DEBUGTARGET) $(RELEASEOBJECTS) $(RELEASETARGET)
