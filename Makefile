CC := gcc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic
DEPFLAGS := -MMD -MP

BUILD_DIR := build
TARGET := $(BUILD_DIR)/3d_shooting
SOURCES := $(wildcard src/*.c) $(wildcard assets/*.c)
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)
TEST_TARGET := $(BUILD_DIR)/test_raycast
TEST_SOURCES := tests/test_raycast.c src/fixed.c src/world.c src/target.c src/player.c src/raycast.c

.PHONY: all check test clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

check:
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsyntax-only $(SOURCES)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES) $(wildcard include/*.h)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPENDENCIES)
