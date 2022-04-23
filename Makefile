### Makefile
### Automates the build and everything else of the project.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include variables.mk

# Directories and Paths
SRCDIR = src
TESTDIR = test
BUILDDIR := build

# Sources and Flags
SOURCES += $(SRCDIR)/pickle.cpp
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))

.PHONY: all compile test debug memcheck clean
all: compile repl

compile: $(BUILDDIR)/stamp $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	$(MKDIR) $(@D)
	$(TOUCH) $@

run: compile test
	cd $(TESTDIR) && $(MAKE) run

debug: CFLAGS += -g3 -DDEBUG
debug: clean compile
	cd $(TESTDIR) && $(MAKE) debug

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean compile
	cd $(TESTDIR) && $(MAKE) memcheck

test: compile
	cd $(TESTDIR) && $(MAKE)

clean:
	$(RM) -r $(BUILDDIR)
	$(RM) valgrind.log
	cd $(TESTDIR) && $(MAKE) clean
