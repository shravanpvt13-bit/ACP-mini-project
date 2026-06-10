# ──────────────────────────────────────────────────────────────────
#  Makefile for 2D Graphics Editor
# ──────────────────────────────────────────────────────────────────

CC       = gcc
CFLAGS   = -Wall -Wextra -std=c11 -O2
LDFLAGS  = -lncurses -lm

# On Windows with PDCurses, uncomment the following line:
# LDFLAGS = -lpdcurses -lm

ifeq ($(OS),Windows_NT)
    CFLAGS += -DNCURSES_STATIC
endif

TARGET   = editor
SOURCES  = main.c shapes.c
HEADERS  = shapes.h
OBJECTS  = $(SOURCES:.c=.o)

# ── Default target ──
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ── Compile source files ──
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# ── Clean build artifacts ──
clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).exe

# ── Run the editor ──
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
