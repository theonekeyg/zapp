CC = /usr/bin/gcc
CFLAGS = -g
SRCS = $(wildcard src/*.c)
OBJS = $(addprefix obj/, $(notdir $(SRCS:.c=.o)))
BIN = main

all: $(BIN)

-include obj/*.d

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

obj/%.o: */%.c
	@mkdir -p obj
	$(CC) -c -o $@ $< -MMD $(CFLAGS)

clean:
	rm -rf obj $(BIN)
