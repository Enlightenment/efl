CC=gcc
TARGET_GTK=gtk-socket
SOURCES_GTK=ta-socket.c \
	ta-socket-label.h \
	ta-socket-label.c \
	ta-socket-accessible.h \
	ta-socket-accessible.c

LIBS_GTK=atk gtk+-3.0
CFLAGS_GTK=`pkg-config --cflags $(LIBS_GTK)`
LDFLAGS_GTK=`pkg-config --libs $(LIBS_GTK)`

TARGET_EFL=efl-plug
SOURCES_EFL=efl-plug.c
LIBS_EFL=elementary
CFLAGS_EFL=`pkg-config --cflags $(LIBS_EFL)`
LDFLAGS_EFL=`pkg-config --libs $(LIBS_EFL)`

all: compile_gtk compile_efl

compile_gtk:
	$(CC) -o $(TARGET_GTK) $(SOURCES_GTK) $(CFLAGS_GTK) $(LDFLAGS_GTK)

compile_efl:
	$(CC) -o $(TARGET_EFL) $(SOURCES_EFL) $(CFLAGS_EFL) $(LDFLAGS_EFL)

clean:
	rm -rf $(TARGET_GTK)
	rm -rf $(TARGET_EFL)
	rm -rf *~
