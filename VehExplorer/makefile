CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Iinc -I../MS_1_files
LDFLAGS :=

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INC_DIR := inc

# Course objects live in the group repo's MS_1_files/ (one level up).
PROVIDER_DIR  := ../MS_1_files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

PROVIDER_OBJS := $(wildcard $(PROVIDER_DIR)/*.o)

TARGET := $(BIN_DIR)/VehExplorer

.PHONY: all clean dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(INC_DIR) input-files

$(TARGET): $(OBJS) $(PROVIDER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(PROVIDER_OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)/VehExplorer
