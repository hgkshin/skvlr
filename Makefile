OBJ_DIR = obj
SRC_DIR = src
INCLUDE_DIR = include
BIN_DIR = bin
LIBS_DIR = libs
TEST_DIR = test/src
PROFILER_DIR = profiler/src
print-%  : ; @echo $* = $($*)

AR = ar -rc
RANLIB = ranlib
CC = g++

CCFLAGS = -ggdb -Wall -Wextra -Werror -Wwrite-strings \
        -O2 -Iinclude -Itest/include -std=c++11 -pthread \
        -I$(LIBS_INCLUDE_DIR)
LDFLAGS = -pthread

SKVLR_SOURCES = skvlr.cc worker.cc murmurhash3.cc
SKVLR_OBJS = $(SKVLR_SOURCES:%.cc=$(OBJ_DIR)/%.o)

TEST_SRCS = skvlr_test.cc test_utils.cc
TEST_OBJS = $(TEST_SRCS:%.cc=$(OBJ_DIR)/%.o)
TEST_BIN = $(BIN_DIR)/test

PROFILER_SRCS = skvlr_profiler.cc
PROFILER_OBJS = $(PROFILER_SRCS:%.cc=$(OBJ_DIR)/%.o)
PROFILER_BIN = $(BIN_DIR)/profiler

LIBRARY_SKVLR = $(LIBS_DIR)/skvlr.a

EXECUTABLES = $(TEST_BIN) $(PROFILER_BIN)

vpath % $(SRC_DIR) $(TEST_DIR) $(PROFILER_DIR)

all: $(EXECUTABLES)

$(LIBRARY_SKVLR): $(SKVLR_OBJS) | $(LIBS_DIR)
	$(AR) $(LIBRARY_SKVLR) $(SKVLR_OBJS)
	$(RANLIB) $(LIBRARY_SKVLR)

$(BIN_DIR):
	@mkdir -p $@

$(LIBS_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	@echo + $@ [cc $<]
	$(CC) $(CCFLAGS) -c $< -o $@ -c

$(TEST_BIN): $(TEST_OBJS) $(LIBRARY_SKVLR) | $(BIN_DIR)
	@echo + $@ [ld $^]
	@$(CC) -o $@ $^ $(LDFLAGS)

test: $(TEST_BIN)
	@-pkill test
	@-$(TEST_BIN)
	@-pkill test

$(PROFILER_BIN): $(PROFILER_OBJS) $(LIBRARY_SKVLR) | $(BIN_DIR)
	@echo + $@ [ld $^]
	@$(CC) -o $@ $^ $(LDFLAGS)

profiler: $(PROFILER_BIN)
	@-$(PROFILER_BIN)

.PHONY: all clean

clean:
	rm $(LIBS_DIR)/* $(OBJ_DIR)/* $(BIN_DIR)/*
