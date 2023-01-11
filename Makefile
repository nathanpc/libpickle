### Makefile
### Automates the build and everything else of the project.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include variables.mk

# Sources and Objects
SOURCES += $(SRCDIR)/pickle.c
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))
TARGET  := $(BUILDDIR)/lib$(PROJECT).a

.PHONY: all compile test debug memcheck clean
all: compile

compile: $(BUILDDIR)/stamp $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/stamp:
	$(MKDIR) $(@D)
	$(TOUCH) $@

run: test

debug: CFLAGS += -g3 -DDEBUG
debug: clean compile
	cd $(TESTDIR) && $(MAKE) debug

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean compile
	cd $(TESTDIR) && $(MAKE) memcheck

test: compile
	cd $(TESTDIR) && $(MAKE) run

clean:
	$(RM) -r $(BUILDDIR)
	cd $(TESTDIR) && $(MAKE) clean
