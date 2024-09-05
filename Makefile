# Makefile for compiling chess.cc and emscripten.cc with Emscripten

# Compiler
EMCC = emcc

# Source files
SRC = src/chess.cc src/emscripten.cc

# Output directory
OUT_DIR = src/wasm

# Output files
OUT_JS = $(OUT_DIR)/chess.js
OUT_WASM = $(OUT_DIR)/chess.wasm

# Exported functions and runtime methods
EXPORTED_FUNCTIONS = '["_startGame", "_askMovements", "_doMovement", "_getChessNotation", "_getStockfishNotation", "_doStockfishMovement", "_doPromote", "_askBoardSquare", "_redoGameMovement", "_undoGameMovement", "_isEnd"]'
EXPORTED_RUNTIME_METHODS = '["UTF8ToString", "ccall", "cwrap"]'

# Compiler flags
CFLAGS = -s EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS) -s EXPORTED_RUNTIME_METHODS=$(EXPORTED_RUNTIME_METHODS)

# Default target
all: $(OUT_JS) $(OUT_WASM)

# Ensure the output directory exists
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Rule to build the output files
$(OUT_JS) $(OUT_WASM): $(SRC) | $(OUT_DIR)
	$(EMCC) -o $(OUT_JS) $(SRC) $(CFLAGS)

# Clean up
clean:
	rm -f $(OUT_JS) $(OUT_WASM)

.PHONY: all clean