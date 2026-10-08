EXEC = dara.out
CC = gcc
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
FLAGS = -g

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $(EXEC)

%.o: %.c include/%.h
	$(CC) -c $(FLAGS) $< -o $@

install:
	make
	cp ./$(EXEC) /usr/local/bin/dara

clean:
	-rm *.out
	-rm *.o
	-rm src/*.o
