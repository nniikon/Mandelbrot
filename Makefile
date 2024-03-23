ExecName = Mandelbrot

all:
	clang++ main.cpp -c -march=znver2 -O3
	clang++ main.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system -lOpenCL -o $(ExecName)

run: all
	./$(ExecName)
