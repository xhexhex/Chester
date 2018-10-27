CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Werror-implicit-function-declaration -Wno-overlength-strings
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

CHESTER_TESTER := tests/ct/chester_tester
CT_SOURCES := $(wildcard tests/ct/*.c)
CT_HEADERS := $(wildcard tests/ct/*.h)
CT_LDFLAGS := -Wl,-rpath=$$HOME/lib -L$$HOME/lib -lchester

.PHONY: ct
ct: $(CHESTER_TESTER)
	@./$(CHESTER_TESTER)

$(CHESTER_TESTER): $(CT_SOURCES) $(CT_HEADERS)
	@gcc $(CFLAGS) -I$$HOME/include -o $(CHESTER_TESTER) $(CT_SOURCES) $(CT_LDFLAGS)

.PHONY: clean
clean:
	@rm --force $(TARGET) $(GEN_CHECKFILES_BIN) $(GEN_CHECKFILES_C) $(CHESTER_TESTER)

.PHONY: showvars
showvars:
	@echo "HEADERS:" $(HEADERS)
	@echo "SOURCES:" $(SOURCES)
	@echo "CHECKFILES:" $(CHECKFILES)
