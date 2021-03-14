CC = /usr/bin/gcc
CFLAGS = -g
SRCS = $(wildcard src/*.c src/*/*.c)
OBJS = $(addprefix obj/, $(notdir $(SRCS:.c=.o)))
BIN = zapp

VPATH = src src/hash

all: $(BIN)

-include obj/*.d

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

obj/%.o: %.c
	@mkdir -p obj
	$(CC) -c -o $@ $< -MMD $(CFLAGS)

clean:
	rm -rf obj $(BIN)

test: $(OBJS)
	cd tests && make
