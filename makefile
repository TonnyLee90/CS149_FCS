CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
TARGET  = fms
SRC     = src/main.c
 
all: $(TARGET)
 
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
 
clean:
	rm -f $(TARGET)
 
run: all
	./$(TARGET)