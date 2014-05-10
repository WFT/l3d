DEPENDENCIES = transform.o parse_util.o display.o render.o matrix.o objects.o
SPECIALFLAGS = `sdl2-config --cflags --static-libs` -lm -g

build:  $(DEPENDENCIES) interpreter.c
	gcc interpreter.c $(DEPENDENCIES) -lSDL2 -g -O3 -o 3D

buildf: $(DEPENDENCIES) interpreter.c
	gcc interpreter.c $(DEPENDENCIES) $(SPECIALFLAGS) -o -O3 3D

transform.o: transform.c transform.h
	gcc transform.c -o transform.o -c

parse_util.o: parse_util.c parse_util.h
	gcc parse_util.c -o parse_util.o -c

objects.o: objects.c objects.h
	gcc objects.c -o objects.o -c

display.o: display.c display.h
	gcc display.c $(SPECIALFLAGS) -o display.o -c

render.o: render.c render.h
	gcc render.c -o render.o -c

matrix.o: matrix.c matrix.h
	gcc matrix.c -o matrix.o -c

clean:
	rm *.o 3D
