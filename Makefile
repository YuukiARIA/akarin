CFLAGS_EXTRA   =
CFLAGS_DEBUG   = -g -DDEBUG
CFLAGS_RELEASE = -O2
CFLAGS         = -I./include -Wall -pedantic-errors $(CFLAGS_EXTRA)
SRCDIRS        = $(shell find src -type d)
OBJDIRS        = $(SRCDIRS:src%=obj%)
DEPSDIRS       = $(SRCDIRS:src%=deps%)
SRCS           = $(shell find src -name '*.c' -type f)
OBJS           = $(SRCS:src/%.c=obj/%.o)
TARGET         = ./bin/akarin
PREFIX         = /usr/local/bin

.PHONY: debug release clean install

debug: CFLAGS += $(CFLAGS_DEBUG)
release: CFLAGS += $(CFLAGS_RELEASE)
debug release: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

obj/%.o: src/%.c
	@mkdir -p $(OBJDIRS) $(DEPSDIRS)
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -MT $@ -MM $< > deps/$*.d

clean:
	$(RM) -rf ./obj ./bin ./deps

install:
	install -m 755 $(TARGET) $(PREFIX)

-include $(shell find deps -name '*.d' -type f 2> /dev/null)
