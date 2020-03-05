CFLAGS:= -Wall -O2 -I/usr/X11R6/include -I../
LINKFLAGS:= -L/usr/X11R6/lib -lX11 -lm
OBJS:= tgaview.o ../gfx.o
HEADS:= tgaview.h ../gfx.h

all: tgaview

tgaview: $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(LINKFLAGS)

%.o: %.c $(HEADS)
	gcc $(CFLAGS) -c -o $@ $<

tgaview_g: $(OBJS)
	gcc $(CFLAGS) $(LINKFLAGS) -ggdb -g -pg -O0 -o $@ $^

prof: tgaview_g
	./tgaview_g && gprof tgaview_g

clean:
	rm -rf *.s *.o tgaview_g*

.PHONY: all prof clean
