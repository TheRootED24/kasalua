NAME ?= kasa
PROG ?= kasa.so
DELETE = rm -rf
OUT ?= -o $(PROG)
LOUT ?= -o $(LPROG)
SOURCES = kasalua.c
CFLAGS = -W -Wall -Wextra -O2
LDFLAGS = -shared -fpic -llua -lm

all: $(NAME)

$(NAME): $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) $(OUT)

clean:
	$(DELETE) *.o *.so
