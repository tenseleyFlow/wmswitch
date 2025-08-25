CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g
INCLUDES = -Iinclude -Ilib
SRCDIR = src
OBJDIR = obj
BINDIR = bin
TARGET = $(BINDIR)/wmswitch

# Find all .c files recursively
SOURCES = $(shell find $(SRCDIR) -name '*.c')
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# TOML library (we'll use tomlc99 - a lightweight C99 TOML parser)
TOML_LIB = lib/tomlc99
TOML_OBJ = $(TOML_LIB)/toml.o
LIBS = 

.PHONY: all clean install test

all: $(TARGET)

$(TARGET): $(OBJECTS) $(TOML_OBJ) | $(BINDIR)
	$(CC) $(OBJECTS) $(TOML_OBJ) -o $@ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TOML_OBJ): $(TOML_LIB)/toml.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
	rm -f $(TOML_OBJ)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

test: $(TARGET)
	@echo "Running tests..."
	@./tests/run_tests.sh

# Development helpers
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

.SUFFIXES:
.INTERMEDIATE: $(OBJECTS)