.PHONY: all
FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0)

all: server

gen:
	gdbus-codegen --generate-c-code dbus-server --c-namespace GawakeServer --interface-prefix io.github.kelvinnovais. io.github.kelvinnovais.xml

server: server.o gawake-dbus.o
	gcc -o $@ $^ $(FLAGS)
