CC = gcc

TARGET = tbot

IDIR = ./include
LDIR = ./lib

CFLAGS = -I$(IDIR)
LIBS = -L$(LDIR)

CFLAGS += $(shell pkg-config --cflags glib-2.0) 
LIBS += $(shell pkg-config --libs glib-2.0) 

# sudo apt-get install libdbus-1-dev
CFLAGS += $(shell pkg-config --cflags dbus-1) 
LIBS += $(shell pkg-config --libs dbus-1) 

OBJ = main.o mainloop.o movement.o

.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -rf $(TARGET) $(OBJ) 