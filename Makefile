ExecName = Mandelbrot

all:
	g++ -c ./Thread-Pool/thread_pool.c -O3
	g++ main.cpp -c -march=znver2 -O3
	g++ thread_pool.o main.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system -lOpenCL -o $(ExecName)

run: all
	./$(ExecName)
