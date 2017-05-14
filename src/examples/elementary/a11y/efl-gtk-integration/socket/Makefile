CC=gcc
TARGET=ta-socket
SOURCES=ta-socket.c \
	ta-socket-label.h \
	ta-socket-label.c \
	ta-socket-accessible.h \
	ta-socket-accessible.c

LIBS=atk gtk+-3.0
CFLAGS=`pkg-config --cflags $(LIBS)`
LDFLAGS=`pkg-config --libs $(LIBS)`

all: compile

compile:
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf $(TARGET)
	rm -rf *~
