EXEC = dara.out
CC = gcc

SRCS = $(shell find src -type f -name '*.c')
OBJS = $(SRCS:.c=.o)

FLAGS = -g
CFLAGS = -std=c99
LIBS = -Iinclude -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

.PHONY: all install clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $@ $(LIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(FLAGS) -Iinclude -c $< -o $@

src/parser/%.o: src/parser/%.c
	$(CC) $(CFLAGS) $(FLAGS) -Iinclude -c $< -o $@

src/visitor/%.o: src/visitor/%.c
	$(CC) $(CFLAGS) $(FLAGS) -Iinclude -c $< -o $@

install: $(EXEC)
	cp ./$(EXEC) /usr/local/bin/dara

clean:
	rm -f $(EXEC) $(OBJS)
