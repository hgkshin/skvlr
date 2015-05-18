OBJ_DIR = obj
SRC_DIR = src
INCLUDE_DIR = include
BIN_DIR = bin
LIBS_DIR = libs
TEST_DIR = test/src

AR = ar -rc
RANLIB = ranlib
CC = g++

CCFLAGS = -ggdb -Wall -Wextra -Werror -Wswitch-default -Wwrite-strings \
        -O2 -Iinclude -Itest/include -std=c++11 \
        -I$(LIBS_INCLUDE_DIR)

SKVLR_SOURCES = skvlr.cc
SKVLR_OBJS = $(SKVLR_SOURCES:%.cc=$(OBJ_DIR)/%.o)

LIBRARY_SKVLR = $(LIBS_DIR)/skvlr.a

vpath % $(SRC_DIR) $(TEST_DIR)

$(LIBRARY_SKVLR): $(SKVLR_OBJS) | $(LIBS_DIR)
	$(AR) $(LIBRARY_SKVLR) $(SKVLR_OBJS)
	$(RANLIB) $(LIBRARY_SKVLR)

all: $(LIBRARY_SKVLR)

$(BIN_DIR):
	@mkdir -p $@

$(LIBS_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	@echo + $@ [cc $<]
	$(CC) $(CCFLAGS) -c $< -o $@ -c

