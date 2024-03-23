ExecName = Mandelbrot

all:
	clang++ main.cpp -c -march=znver2 -fsanitize=address -O3
	clang++ main.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system -lOpenCL -fsanitize=address -o $(ExecName)

run: all
	./$(ExecName)
