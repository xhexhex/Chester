CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Werror-implicit-function-declaration
TARGET := libchester.so
SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)
# OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
CHECKFILES := $(wildcard tests/*.check)
# GEN as in (automatically) generated
GEN_CHECKFILES_C := $(patsubst %.check,%.c,$(CHECKFILES))
GEN_CHECKFILES_BIN := $(patsubst %.check,%,$(CHECKFILES))

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	@gcc $(CFLAGS) -shared -o $(TARGET) -fPIC $(SOURCES)
	@if [ -d $$HOME/include ]; then cp src/chester.h $$HOME/include; fi
	@if [ -d $$HOME/lib ]; then cp $(TARGET) $$HOME/lib; fi

.PHONY: check
check: $(GEN_CHECKFILES_BIN)
	@for ts in $(GEN_CHECKFILES_BIN); do ./$$ts; done

$(GEN_CHECKFILES_BIN): %: %.c $(HEADERS) $(SOURCES)
	@gcc $(CFLAGS) -pthread -o $@ $< $(SOURCES) -lcheck -lm -lrt -lsubunit

$(GEN_CHECKFILES_C): %.c: %.check
	@checkmk $< > $@

.PHONY: clean
clean:
	@rm --force $(TARGET) $(GEN_CHECKFILES_BIN) $(GEN_CHECKFILES_C)

.PHONY: showvars
showvars:
	@echo "HEADERS:" $(HEADERS)
	@echo "SOURCES:" $(SOURCES)
	@echo "CHECKFILES:" $(CHECKFILES)
