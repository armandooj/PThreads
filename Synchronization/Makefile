CC=gcc
CFLAGS=-g -Wall -Werror
LDFLAGS=-lpthread 

SOURCES=linked_list.c reader_writer_tracing.c 
READER_WRITER=$(wildcard reader_writer_?.c)

OBJS=$(SOURCES:.c=.o) 


TEST_SRC=$(wildcard *_test.c)

TEST_BIN+=$(TEST_SRC:.c=_0)

#Uncomment when you wrote reader_writer_1.c (part 1)
TEST_BIN+=$(TEST_SRC:.c=_1)

#Uncomment when you wrote reader_writer_2.c (part 2)
#TEST_BIN+=$(TEST_SRC:.c=_2)

#Uncomment when you wrote reader_writer_3.c (part 3)
#TEST_BIN+=$(TEST_SRC:.c=_3)

build_all_tests:$(TEST_BIN)

%_test_0:%_test.o $(OBJS) reader_writer_0.o
	$(CC) $(LDFLAGS) $^ -o $@

%_test_1:%_test.o $(OBJS) reader_writer_1.o
	$(CC) $(LDFLAGS) $^ -o $@

%_test_2:%_test.o $(OBJS) reader_writer_2.o
	$(CC) $(LDFLAGS) $^ -o $@

%_test_3:%_test.o $(OBJS) reader_writer_3.o
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf	echo *.o $(TEST_BIN)