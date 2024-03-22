ExecName = Mandelbrot

all:
	g++ main.cpp -c -O3 -march=znver2
	g++ main.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system -o $(ExecName)

run: all
	./$(ExecName)
