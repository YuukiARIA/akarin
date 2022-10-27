CFLAGS  = -Wall -O2
SRCS    = $(wildcard src/*.c)
OBJS    = $(SRCS:src/%.c=obj/%.o)
TARGET  = ./bin/akarin
PREFIX  = /usr/local/bin

.PHONY: clean install

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(OBJS) -o $(TARGET)

obj/%.o: src/%.c
	@mkdir -p obj deps
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) -MT $@ -MM $< > deps/$*.d

clean:
	$(RM) -rf ./obj ./bin ./deps

install:
	install -m 755 $(TARGET) $(PREFIX)

-include $(wildcard ./deps/*.d)
