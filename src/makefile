DEPENDENCIES = transform.o parse_util.o display.o render.o matrix.o objects.o lines.o
SPECIALFLAGS = `sdl2-config --cflags --static-libs` -lm
STDFLAGS = -g -O3

build:  $(DEPENDENCIES) interpreter.c
	gcc interpreter.c $(DEPENDENCIES) -lSDL2 -o 3D $(STDFLAGS)

buildf: $(DEPENDENCIES) interpreter.c
	gcc interpreter.c $(DEPENDENCIES) $(SPECIALFLAGS) -o 3D $(STDFLAGS)

transform.o: transform.c transform.h
	gcc transform.c -o transform.o -c $(STDFLAGS)

parse_util.o: parse_util.c parse_util.h
	gcc parse_util.c -o parse_util.o -c $(STDFLAGS)

objects.o: objects.c objects.h
	gcc objects.c -o objects.o -c $(STDFLAGS)

display.o: display.c display.h
	gcc display.c -o display.o -c $(STDFLAGS) $(SPECIALFLAGS)

render.o: render.c render.h
	gcc render.c -o render.o -c $(STDFLAGS)

matrix.o: matrix.c matrix.h
	gcc matrix.c -o matrix.o -c $(STDFLAGS)

lines.o: lines.c lines.h
	gcc lines.c -o lines.o -c $(STDFLAGS)

clean:
	rm *.o 3D
