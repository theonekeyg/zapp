TESTS!= echo *.c
OBJS = $(addprefix ../obj/, misc.o parse.o tokenize.o ast.o hashtable.o)
INCLUDE = -I../src

.PHONY: $(TESTS)

all: $(TESTS)

$(TESTS):
	@$(CC) -o $*.exe $*.c $(CFLAGS) $(INCLUDE) $(OBJS)
	@./$*.exe
	@echo "\033[32m\`$*\` test successfully passed\033[m"
