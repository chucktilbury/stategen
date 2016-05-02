
# The file scanner.c is generated from the file scanner.sm.  A working scanner.c
# that has been hardcoded is stored in the backups directory.

OBJS 		=	files.o \
			hashtable.o \
			scanner.o \
			symbols.o \
			tokens.o \
			parse.o \
			emit.o \
			validate.o \
			errors.o

			#main.o

HEADERS	= $(OBJS:%.o=%.h)
EXEC_XTN	=	.exe
STATEGEN		= 	stategen$(EXEC_XTN)

TESTS		= 	./tests
UNIT_TESTS 	= 	$(TESTS)/hash_utest$(EXEC_XTN) \
			$(TESTS)/scan_utest$(EXEC_XTN) \
			$(TESTS)/tok_utest$(EXEC_XTN) \
			$(TESTS)/parse_utest$(EXEC_XTN) \
			$(TESTS)/emit_utest$(EXEC_XTN)
CARGS		= -Wall -g

.c.o: $(HEADERS)
	gcc $(CARGS) -c $< -o $@

all: $(STATEGEN)
tests: $(UNIT_TESTS)

$(STATEGEN): $(OBJS) $(HEADERS) main.c
	gcc $(CARGS) -o $(STATEGEN) main.c $(OBJS)

$(TESTS)/hash_utest$(EXEC_XTN): $(OBJS) $(HEADERS)
	gcc $(CARGS) -o $(TESTS)/hash_utest$(EXEC_XTN) $(OBJS:hashtable.o=hashtable.c) -DUNIT_TEST

$(TESTS)/scan_utest$(EXEC_XTN): $(OBJS) $(HEADERS) scan_test.c
	gcc $(CARGS) -o $(TESTS)/scan_utest$(EXEC_XTN) $(OBJS:scanner.o=scan_test.c) -DDEBUGGING -DUNIT_TEST

$(TESTS)/tok_utest$(EXEC_XTN): $(OBJS) $(HEADERS)
	gcc $(CARGS) -o $(TESTS)/tok_utest$(EXEC_XTN) $(OBJS:tokens.o=tokens.c) -DUNIT_TEST

$(TESTS)/parse_utest$(EXEC_XTN): $(OBJS) $(HEADERS) parse_test.c
	gcc $(CARGS) -o $(TESTS)/parse_utest$(EXEC_XTN) $(OBJS:parse.o=parse_test.c) -DUNIT_TEST

$(TESTS)/emit_utest$(EXEC_XTN): $(OBJS) $(HEADERS)
	gcc $(CARGS) -o $(TESTS)/emit_utest$(EXEC_XTN) $(OBJS:emit.o=emit.c) -DUNIT_TEST

parse_test.c: $(STATEGEN) sm/parse.sm
	./stategen -i:sm/parse.sm -o:parse_test.c

scan_test.c: $(STATEGEN) sm/scanner.sm
	./stategen -i:sm/scanner.sm -o:scan_test.c

clean:
	rm -f $(OBJS) $(UNIT_TESTS) $(SCANGEN) main.o scan_test.c parse_test.c
