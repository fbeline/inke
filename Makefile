CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Wno-unused-parameter \
         -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments
SANITIZE_FLAGS = -fsanitize=undefined,address

DEBUG_DIR = build/debug
RELEASE_DIR = build/release

# Automatically find all .c files in src/ directory
SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRC:$(SRC_DIR)/%.c=%.o)

DEBUG_OBJS = $(addprefix $(DEBUG_DIR)/, $(OBJS))
RELEASE_OBJS = $(addprefix $(RELEASE_DIR)/, $(OBJS))

TARGET = inke

all: debug

debug: CFLAGS += $(SANITIZE_FLAGS) -g
debug: $(DEBUG_DIR)/$(TARGET)

release: CFLAGS += -O2
release: $(RELEASE_DIR)/$(TARGET)

$(DEBUG_DIR)/$(TARGET): $(DEBUG_OBJS)
	$(CC) $(CFLAGS) -o $@ $(DEBUG_OBJS)

$(RELEASE_DIR)/$(TARGET): $(RELEASE_OBJS)
	$(CC) $(CFLAGS) -o $@ $(RELEASE_OBJS)

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(DEBUG_DIR):
	mkdir -p $(DEBUG_DIR)

$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)

gdb: debug
	gdb $(DEBUG_DIR)/$(TARGET)

run: debug
	./$(DEBUG_DIR)/$(TARGET) notes.txt

clean:
	rm -rf build/

.PHONY: all debug release clean gdb run

