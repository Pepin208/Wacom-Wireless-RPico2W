CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lws2_32

TARGET = wacom_wifi_daemon.exe
OBJS = main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET) $(OBJS)
