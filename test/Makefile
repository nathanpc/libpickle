### Makefile
### Automates the build of the example program.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include ../variables.mk

# Directories and Paths
LIBDIR      := ../$(SRCDIR)
PRJBUILDDIR := ../$(BUILDDIR)
TARGET      := $(PRJBUILDDIR)/pickle_test

# Sources and Objects
SOURCES  = main.c
OBJECTS := $(addprefix $(PRJBUILDDIR)/, $(patsubst %.c, %.o, $(SOURCES)))

.PHONY: all compile run debug memcheck clean
all: compile

compile: $(TARGET)

$(TARGET): $(OBJECTS) $(PRJBUILDDIR)/pickle.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(PRJBUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g3 -DDEBUG
debug: clean compile
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean compile
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all \
		--track-origins=yes --log-file=$(PRJBUILDDIR)/valgrind.log $(TARGET)
	cat $(PRJBUILDDIR)/valgrind.log

run: compile
	$(TARGET)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
	$(RM) $(PRJBUILDDIR)/valgrind.log
