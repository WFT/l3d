3DOSX: transform.o parse_util.o render.o matrix.o lines3Dinterpreter.c
	gcc lines3Dinterpreter.c transform.o parse_util.o render.o matrix.o -lSDL2 -o 3D

ubuntu: transform.o parse_util.o render.o matrix.o lines3Dinterpreter.c
	gcc lines3Dinterpreter.c transform.o parse_util.o render.o matrix.o -I/usr/local/include/SDL2 -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL2 -lm -o 3D

transform.o: transform.c transform.h
	gcc transform.c -o transform.o -c

parse_util.o: parse_util.c parse_util.h
	gcc parse_util.c -o parse_util.o -c

render.o: render.c render.h
	gcc render.c -o render.o -c

matrix.o: matrix.c matrix.h
	gcc matrix.c -o matrix.o -c

clean:
	rm *.o 3D *~
