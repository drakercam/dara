EXEC = dara.out
CC = gcc

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

FLAGS = -g
CPPFLAGS = -Iinclude

.PHONY: all install clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $@

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(FLAGS) -c $< -o $@

install: $(EXEC)
	cp ./$(EXEC) /usr/local/bin/dara

clean:
	rm -f $(EXEC) $(OBJS)
