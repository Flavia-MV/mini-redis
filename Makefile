CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src

CORE_SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/server.c $(SRC_DIR)/parser.c \
			$(SRC_DIR)/hashtable.c $(SRC_DIR)/persistence.c

.PHONY: all test clean

all: mini_redis

mini_redis: $(CORE_SRCS)
	$(CC) $(CFLAGS) -o $@ $(CORE_SRCS)

test_parser: $(SRC_DIR)/test_parser.c $(SRC_DIR)/parser.c
	$(CC) $(CFLAGS) -o $@ $(SRC_DIR)/test_parser.c $(SRC_DIR)/parser.c

test_hashtable: $(SRC_DIR)/test_hashtable.c $(SRC_DIR)/hashtable.c
	$(CC) $(CFLAGS) -o $@ $(SRC_DIR)/test_hashtable.c $(SRC_DIR)/hashtable.c

test_server: $(SRC_DIR)/test_server.c $(SRC_DIR)/server.c
	$(CC) $(CFLAGS) -o $@ $(SRC_DIR)/test_server.c $(SRC_DIR)/server.c

test: test_parser test_hashtable test_server
	./test_parser
	./test_hashtable
	./test_server

clean:
	rm -f mini_redis test_parser test_hashtable test_server *.aof