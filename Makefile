ExecName = Mandelbrot

all:
	g++ main.cpp -c -O3 -mavx
	g++ main.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system -o $(ExecName)

run: all
	./$(ExecName)
