
EXEC = dara.out
CC = gcc

CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L

DEBUG_FLAGS = -g -DPROFILE
RELEASE_FLAGS = -g

SRCS = $(shell find src -type f -name '*.c')

DEBUG_OBJS = $(patsubst src/%.c,build/debug/%.o,$(SRCS))
RELEASE_OBJS = $(patsubst src/%.c,build/release/%.o,$(SRCS))

LIBS = -Iinclude -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

.PHONY: all debug release install clean

# Default build
all: release

# Profile-enabled debug build
debug: $(DEBUG_OBJS)
	$(CC) $(DEBUG_OBJS) $(DEBUG_FLAGS) -o $(EXEC) $(LIBS)

# Normal build without profiling
release: $(RELEASE_OBJS)
	$(CC) $(RELEASE_OBJS) $(RELEASE_FLAGS) -o $(EXEC) $(LIBS)

# Debug compilation
build/debug/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -Iinclude -c $< -o $@

# Release compilation
build/release/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -Iinclude -c $< -o $@

install: release
	cp ./$(EXEC) /usr/local/bin/dara

clean:
	rm -rf build $(EXEC)
