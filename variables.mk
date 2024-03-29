### variables.mk
### Common variables used throughout the project.
###
### Author: Nathan Campos <nathan@innoveworkshop.com>

# Project
PROJECT = pickle

# Environment
PLATFORM := $(shell uname -s)

# Directories and Paths
SRCDIR     := src
TESTDIR    := test
BUILDDIR   := build
PKLEXAMPLE ?= example.pkl

# Tools
CC    = gcc
AR    = ar
GDB   = gdb
RM    = rm -f
MKDIR = mkdir -p
TOUCH = touch

# Handle OS X-specific tools.
ifeq ($(PLATFORM), Darwin)
	CC  = clang
	GDB = lldb
endif

# Flags
CFLAGS  = -Wall -Wno-psabi --std=c89
LDFLAGS =
