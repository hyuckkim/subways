CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -O2
SRCDIR = src
INCDIR = include
BINDIR = .

# Target executables
TARGET = subways
CHECK_TARGET = check_c

# Common source files
COMMON_SRCS = $(SRCDIR)/parser.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)

# Main app source files
APP_SRCS = $(SRCDIR)/main.c $(SRCDIR)/pq.c $(SRCDIR)/search.c
APP_OBJS = $(APP_SRCS:.c=.o)

# Checker source files
CHECK_SRCS = $(SRCDIR)/check.c
CHECK_OBJS = $(CHECK_SRCS:.c=.o)

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(APP_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

check: $(CHECK_TARGET)
	./$(CHECK_TARGET)

$(CHECK_TARGET): $(CHECK_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(INCDIR)/subways.h $(INCDIR)/pq.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o $(TARGET) $(CHECK_TARGET)
