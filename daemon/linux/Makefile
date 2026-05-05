CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGET = wacom_wifi_daemon
OBJS = main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
