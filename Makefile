EXEC = dara.out
CC = gcc

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

FLAGS = -g
CFLAGS = -std=c99 -Iinclude -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

.PHONY: all install clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(FLAGS) -c $< -o $@

install: $(EXEC)
	cp ./$(EXEC) /usr/local/bin/dara

clean:
	rm -f $(EXEC) $(OBJS)
