CC = gcc
CFLAGS = -Iinclude `pkg-config --cflags gtk+-3.0 gtksourceview-3.0` -g -Wall
LIBS = `pkg-config --libs gtk+-3.0 gtksourceview-3.0`

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

SOURCES = $(shell find $(SRC_DIR) -name '*.c')
RESOURCE_SRC = $(BUILD_DIR)/radoide-resources.c
RESOURCE_XML = radoide.gresource.xml

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) $(BUILD_DIR)/radoide-resources.o

TARGET = $(BUILD_DIR)/radoide.exe

.DEFAULT_GOAL := all

all: $(TARGET)

$(RESOURCE_SRC): $(RESOURCE_XML) radoidelogo.jpg
	@echo "Generating resources..."
	@mkdir -p $(BUILD_DIR)
	glib-compile-resources $(RESOURCE_XML) --target=$(RESOURCE_SRC) --generate-source --c-name radoide_resources

$(BUILD_DIR)/radoide-resources.o: $(RESOURCE_SRC)
	@echo "Compiling resources..."
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	$(CC) -o $@ $^ $(LIBS)
	@echo "Build complete! Run with: ./$(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

run: $(TARGET)
	./$(TARGET)

help:
	@echo "Available targets:"
	@echo "  all    - Build the project (default)"
	@echo "  clean  - Remove build files"
	@echo "  run    - Build and run the program"
	@echo "  help   - Show this help"

.PHONY: all clean run help