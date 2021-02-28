CC			= gcc
CFLAGS			= -c -Wall -Wextra -Wfatal-errors -Wno-unused-function
SRCDIR 			= .
DEBUGOUTPUTDIR 		= build/debug
RELEASEOUTPUTDIR	= build/release
EXAMPLE_UDP_SOURCES	= cd_example_udp.c
INCLUDES		= -I. -I../include
LIBS			= -lcd -pthread
_EXAMPLE_UDP_OBJECTS		= $(EXAMPLE_UDP_SOURCES:.c=.o)
EXAMPLE_UDP_DEBUGOBJECTS 	= $(patsubst %,$(DEBUGOUTPUTDIR)/%,$(_EXAMPLE_UDP_OBJECTS))
EXAMPLE_UDP_RELEASEOBJECTS 		= $(patsubst %,$(RELEASEOUTPUTDIR)/%,$(_EXAMPLE_UDP_OBJECTS))
EXAMPLE_UDP_DEBUGTARGET		= build/debug/cdexampleudp
EXAMPLE_UDP_RELEASETARGET	= build/release/cdexampleudp

debugprereqs:
		mkdir -p $(DEBUGOUTPUTDIR)

releaseprereqs:
		mkdir -p $(RELEASEOUTPUTDIR)

debugall:	debugprereqs $(EXAMPLE_UDP_SOURCES) $(EXAMPLE_UDP_DEBUGTARGET)
releaseall:	releaseprereqs $(EXAMPLE_UDP_SOURCES) $(EXAMPLE_UDP_RELEASETARGET)

# additional flags
# CONFIG_DEBUG_LIST	- extensive debugging of list with external debugging
# 			functions
examples-debug:		CFLAGS += -DDEBUG -g -ggdb3 -O0
examples-debug:		debugall

examples-release:	CFLAGS +=
examples-release: 	releaseall

examples:		examples-release


$(EXAMPLE_UDP_DEBUGTARGET): $(EXAMPLE_UDP_DEBUGOBJECTS) 
	$(CC) $(LDFLAGS) $(EXAMPLE_UDP_DEBUGOBJECTS) $(LIBS) -o $@

$(EXAMPLE_UDP_RELEASETARGET): $(EXAMPLE_UDP_RELEASEOBJECTS) 
	$(CC) $(LDFLAGS) $(EXAMPLE_UDP_RELEASEOBJECTS) $(LIBS) -o $@


$(DEBUGOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(RELEASEOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

all: examples-release

.DEFAULT_GOAL = examples-release

clean:
	rm -rf $(EXAMPLE_UDP_DEBUGOBJECTS) $(EXAMPLE_UDP_DEBUGTARGET)
	rm -rf $(EXAMPLE_UDP_RELEASEOBJECTS) $(EXAMPLE_UDP_RELEASETARGET)
