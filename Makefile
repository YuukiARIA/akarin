CFLAGS         = -Wall -pedantic-errors
CFLAGS_DEBUG   = -g -DDEBUG
CFLAGS_RELEASE = -O2
SRCDIRS        = $(shell find src -type d)
OBJDIRS        = $(SRCDIRS:src%=obj%)
DEPSDIRS       = $(SRCDIRS:src%=deps%)
SRCS           = $(shell find src -name '*.c' -type f)
OBJS           = $(SRCS:src/%.c=obj/%.o)
TARGET         = ./bin/akarin
PREFIX         = /usr/local/bin

.PHONY: debug release dirs clean install

debug: CFLAGS += $(CFLAGS_DEBUG)
release: CFLAGS += $(CFLAGS_RELEASE)
debug release: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

obj/%.o: src/%.c dirs
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) -MT $@ -MM $< > deps/$*.d

dirs:
	mkdir -p $(OBJDIRS) $(DEPSDIRS)

clean:
	$(RM) -rf ./obj ./bin ./deps

install:
	install -m 755 $(TARGET) $(PREFIX)

-include $(wildcard ./deps/*.d)
