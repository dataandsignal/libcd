CC			= gcc
CFLAGS			= -c -Wall -Wextra -Wfatal-errors -Wno-unused-function
SRCDIR 			= .
DEBUGOUTPUTDIR 		= build/debug
RELEASEOUTPUTDIR	= build/release
SOURCES			= cd_test_list.c
INCLUDES		= -I. -I../include
_OBJECTS		= $(SOURCES:.c=.o)
DEBUGOBJECTS 		= $(patsubst %,$(DEBUGOUTPUTDIR)/%,$(_OBJECTS))
RELEASEOBJECTS 		= $(patsubst %,$(RELEASEOUTPUTDIR)/%,$(_OBJECTS))
DEBUGTARGET		= build/debug/cdtestlist
RELEASETARGET		= build/release/cdtestlist

debugall:	$(SOURCES) $(DEBUGTARGET)
releaseall:	$(SOURCES) $(RELEASETARGET)

# additional flags
# CONFIG_DEBUG_LIST	- extensive debugging of list with external debugging
# 			functions
test-debug:		CFLAGS += -DDEBUG -g -ggdb3 -O0
test-debug:		debugall

test-release:	CFLAGS += -DNDEBUG
test-release: 	releaseall

test:		test-release
		./$(RELEASETARGET)

$(DEBUGTARGET): $(DEBUGOBJECTS) 
	$(CC) $(LDFLAGS) $(DEBUGOBJECTS) -o $@

$(RELEASETARGET): $(RELEASEOBJECTS) 
	$(CC) $(LDFLAGS) $(RELEASEOBJECTS) -o $@

$(DEBUGOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(RELEASEOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf $(DEBUGOBJECTS) $(DEBUGTARGET) $(RELEASEOBJECTS) $(RELEASETARGET)
