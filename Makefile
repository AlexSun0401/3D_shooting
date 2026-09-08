CC := gcc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic
DEPFLAGS := -MMD -MP

BUILD_DIR := build
TARGET := $(BUILD_DIR)/3d_shooting
SOURCES := $(wildcard src/*.c) $(wildcard assets/*.c)
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPENDENCIES := $(OBJECTS:.o=.d)

.PHONY: all check clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

check:
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsyntax-only $(SOURCES)

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPENDENCIES)
