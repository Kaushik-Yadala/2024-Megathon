# Compiler and flags
CC = gcc
CFLAGS = -Wall -pthread -fPIC
LDFLAGS = -L. -lnetwork

# Targets
TARGET = run
LIB_STATIC = libnetwork.a
LIB_SHARED = libnetwork.so

# Source files
LIB_SRC = network.c
LIB_OBJ = network.o
MAIN_SRC = run.c

# Default target
all: $(TARGET)

# Rule to build the main executable with the shared library
$(TARGET): $(MAIN_SRC) $(LIB_SHARED)
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN_SRC) $(LDFLAGS)

# Rule to create the shared library
$(LIB_SHARED): $(LIB_OBJ)
	$(CC) -shared -o $(LIB_SHARED) $(LIB_OBJ)

# Rule to create the static library
$(LIB_STATIC): $(LIB_OBJ)
	ar rcs $(LIB_STATIC) $(LIB_OBJ)

# Compile the object file from the library source
$(LIB_OBJ): $(LIB_SRC) network.h
	$(CC) $(CFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)

# Clean up all generated files
clean:
	rm -f $(TARGET) $(LIB_OBJ) $(LIB_SHARED) $(LIB_STATIC)

.PHONY: all clean
