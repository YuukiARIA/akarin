CC      = gcc
CFLAGS  = -Wall -O2
SRCS    = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJS    = $(SRCS:src/%.c=obj/%.o)
TARGET  = ./bin/akarin
DEPS    = .depends.inc

.PHONY: clean

$(TARGET): $(OBJS) bin/
	$(CC) $(OBJS) -o $(TARGET)

obj/%.o: src/%.c obj/
	$(CC) $(CFLAGS) -c $< -o $@

obj/ bin/:
	mkdir $@

$(DEPS): $(SRCS) $(HEADERS)
	$(CC) -MMD $(SRCS) > $(DEPS)

clean:
	$(RM) -rf ./obj ./bin $(DEPS)

-include $(DEPS)
