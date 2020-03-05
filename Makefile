CFLAGS:= -Wall -O2 -I/usr/X11R6/include
LINKFLAGS:= -L/usr/X11R6/lib -lX11 -lm
OBJS:= main.o file.o math.o objparser.o tgaparser.o  gfx.o
HEADS:= rend.h file.h math.h objparser.h tgaparser.h  gfx.h

rend: $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(LINKFLAGS)

all: rend tgaview

tgaview:
	cd tgaview && make

%.o: %.c $(HEADS)
	gcc $(CFLAGS) -c -o $@ $<

rend_g: $(OBJS)
	gcc $(CFLAGS) $(LINKFLAGS) -ggdb -g -pg -O0 -o $@ $^

prof: rend_g
	./rend_g && gprof rend_g

clean:
	rm -rf *.s *.o rend_g*

.PHONY: all prof clean tgaview
