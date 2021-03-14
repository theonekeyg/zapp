TESTS!= echo *.c
OBJS = $(addprefix ../src/, misc.o parse.o tokenize.o ast.o hash/hashtable.o)
INCLUDE = -I../include

.PHONY: $(TESTS)

all: $(TESTS)

$(TESTS):
	@$(CC) -o $*.exe $*.c $(CFLAGS) $(INCLUDE) $(OBJS)
	@./$*.exe
	@echo "\033[32m\`$*\` test successfully passed\033[m"
