### Makefile
### Automates the build of the example program.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

include ../variables.mk

# Sources and Flags
OBJECTS := main.o
LIBDIR := ../src
TARGET = pickle_test
CFLAGS = -Wall -Wno-psabi
LDFLAGS =

.PHONY: all compile run debug memcheck clean
all: compile

compile: $(TARGET)

$(TARGET): $(OBJECTS) # pickle.o
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

debug: CFLAGS += -g3 -DDEBUG
debug: clean compile
	$(GDB) $(TARGET)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: clean compile
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all \
		--track-origins=yes --log-file=valgrind.log $(TARGET)
	cat valgrind.log

run: compile
	./$(TARGET)

clean:
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) valgrind.log
