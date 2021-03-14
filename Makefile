CC = /usr/bin/gcc
CFLAGS = -g
INCLUDE = -I./include
SRCS = $(wildcard src/*.c src/*/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
BIN = zapp

VPATH = src src/hash

all: $(BIN)

-include $(DEPS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< -MMD $(CFLAGS) $(INCLUDE)

clean:
	rm -rf $(BIN) $(OBJS) $(DEPS)

test: $(OBJS)
	cd tests && make
