DEPENDENCIES = transform.o parse_util.o display.o render.o matrix.o objects.o
SPECIALFLAGS = `sdl2-config --cflags --static-libs` -lm -g

build:  $(DEPENDENCIES) interpreter.c
	gcc interpreter.c $(DEPENDENCIES) -lSDL2 -g -O3 -o 3D

buildf: $(DEPENDENCIES) interpreter.c
	gcc interpreter.c $(DEPENDENCIES) $(SPECIALFLAGS) -O3 -o 3D

transform.o: transform.c transform.h
	gcc transform.c -o transform.o -c -g

parse_util.o: parse_util.c parse_util.h
	gcc parse_util.c -o parse_util.o -c -g

objects.o: objects.c objects.h
	gcc objects.c -o objects.o -c -g

display.o: display.c display.h
	gcc display.c $(SPECIALFLAGS) -o display.o -c -g

render.o: render.c render.h
	gcc render.c -o render.o -c -g

matrix.o: matrix.c matrix.h
	gcc matrix.c -o matrix.o -c -g

clean:
	rm *.o 3D
