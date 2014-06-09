DEPENDENCIES = transform.o parse_util.o display.o render.o matrix.o objects.o lines.o
SDLFLAGS = `sdl2-config --cflags --static-libs`
STDFLAGS = -g -O3

build: $(DEPENDENCIES) interpreter.c
	gcc interpreter.c $(DEPENDENCIES) $(STDFLAGS) $(SDLFLAGS) -lm -o 3D 

run: build
	./3D ../examples/tinycube.l3d

transform.o: transform.c transform.h
	gcc transform.c -o transform.o -c $(STDFLAGS)

parse_util.o: parse_util.c parse_util.h
	gcc parse_util.c -o parse_util.o -c $(STDFLAGS)

objects.o: objects.c objects.h
	gcc objects.c -o objects.o -c $(STDFLAGS)

display.o: display.c display.h
	gcc display.c -o display.o -c $(STDFLAGS) $(SDLFLAGS)

render.o: render.c render.h options.h
	gcc render.c -o render.o -c $(STDFLAGS)

matrix.o: matrix.c matrix.h
	gcc matrix.c -o matrix.o -c $(STDFLAGS)

lines.o: lines.c lines.h options.h
	gcc lines.c -o lines.o -c $(STDFLAGS)

test: test.c matrix.o transform.o display.o render.o lines.o
	gcc test.c display.o render.o matrix.o transform.o lines.o -o test $(SDLFLAGS) $(STDFLAGS)

clean:
	rm -r *.o 3D*
