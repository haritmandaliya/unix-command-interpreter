#==============================================================================
# Makefile for compilation of the UNIX Command Interpreter
# 
# Defines variables for compiler, flags, source files, object files, 
# and target executable. Provides rules for building, running, cleaning, 
# and debugging.
#==============================================================================

# Compiler to use
CC = gcc

# Compiler flags:
#   -Wall    : Enable all standard compiler warnings.
#   -Wextra  : Enable extra compiler warnings for higher code quality.
#   -pedantic: Issue all warnings demanded by strict ISO C.
#   -std=c11 : Ensure compliance with the C11 standard.
#   -Iinclude: Instruct the preprocessor to search the 'include' directory for headers.
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Iinclude

# List of source C files to be compiled (obsolete sequential.c is excluded)
SRC = src/main.c src/tokenizer.c src/parser.c src/execute.c src/builtin.c src/history.c

# List of object files (.o) created by replacing '.c' with '.o' in SRC list
OBJ = $(SRC:.c=.o)

# Name of the final executable file
TARGET = prog01

# 'all' is the default target that gets executed when you type 'make' or 'make all'
all: $(TARGET)

# Rule to link the target executable from the compiled object files
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# Pattern rule to compile each individual '.c' file into a '.o' object file
#   $< : represents the dependency (the .c source file)
#   $@ : represents the target (the .o object file)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 'clean' target deletes all generated object files and the executable
# to start compilation fresh
clean:
	rm -f $(OBJ) $(TARGET)

# 'run' target builds the binary and then immediately executes it
run: all
	./$(TARGET)

# 'debug' target adds debug flags (-g -O0) to enable Valgrind/GDB tracing,
# then performs a clean build
debug: CFLAGS += -g -O0
debug: clean all

# Declares targets as phony to prevent conflicts with local files of the same name
.PHONY: all clean run debug
