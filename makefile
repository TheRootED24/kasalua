PROG ?= kasa.so
LPROG ?= libkasa.so
DELETE = rm -rf
OUT ?= -o $(PROG)
LOUT ?= -o $(LPROG)
SOURCES = kasalua.c
CFLAGS = -W -Wall -Wextra -g -O2
LDFLAGS = -shared -fpic -llua -lm
LUAEXC = lua

all: $(PROG)

$(PROG): $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) $(OUT)
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) $(LOUT)

clean:
	$(DELETE) $(PROG) *.o *.obj *.exe *.dSYM
