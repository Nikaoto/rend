CC:= gcc
CFLAGS:= -std=c99 -pedantic -Wall -O2 -I/usr/X11R6/include
LINKFLAGS:= -L/usr/X11R6/lib -lX11 -lm
OBJS_DIR:= .objs
OBJS:= main.o file.o math.o objparser.o tgaparser.o  gfx.o
HEADS:= rend.h file.h math.h objparser.h tgaparser.h  gfx.h
OBJS:= $(addprefix $(OBJS_DIR)/, $(OBJS))

$(shell mkdir -p $(OBJS_DIR))

rend: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LINKFLAGS)

all: rend tgaview

tgaview:
	cd tgaview && make

$(OBJS_DIR)/%.o: %.c $(HEADS)
	$(CC) $(CFLAGS) -c -o $@ $<

rend_g: $(OBJS)
	$(CC) $(CFLAGS) $(LINKFLAGS) -ggdb -g -pg -O0 -o $@ $^

prof: rend_g
	./rend_g && gprof rend_g

clean:
	rm -rf *.s $(OBJS) rend_g*

.PHONY: all prof clean tgaview
