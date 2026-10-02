#config
CC      = gcc
CFLAGS  = -Wall -Wextra -g       
TARGET  = shell

SRCS    = $(wildcard *.c)
OBJS    = $(SRCS:.c=.o)
HEADERS = $(wildcard *.h)

#targets
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

debug: all
	gdb ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run debug clean
