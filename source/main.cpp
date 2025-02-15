#include <SFML/Graphics.hpp>

#include <cassert>
#include <cmath>
#include <cstring>
#include <immintrin.h>

#include "mandelbrot_naive.h"
#include "mandelbrot_vectorized.h"
#include "mandelbrot_arrayed.h"
#include "mandelbrot_openmp.h"
#include "mandelbrot_thread_pool.h"
#ifdef GPU
#include <cuda_runtime.h>
#include "mandelbrot_cuda.h"
#endif

#include "mandelbrot_config.h"

static void test_implementation(sf::Uint8* pixels, void (*func)(sf::Uint8*, float, float),
                                const char* name, int nTests)
{
    printf("Testing %d times %s\n", nTests, name);

    for (int i = 0; i < nTests; i++)
        func(pixels, 1.0f, 0.0f);
}

int main(int argc, char* argv[])
{
    sf::Uint8* pixels = (sf::Uint8*) calloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4,
                                                            sizeof(sf::Uint8));
    if (argc == 3)
    {
        int nTests = 0;
        sscanf(argv[2], "%d", &nTests);
#ifdef GPU
        if (strcmp(argv[1], "cuda_no_cpy") == 0) {
            sf::Uint8* d_pixels;
            size_t size = WINDOW_WIDTH * WINDOW_HEIGHT * 4 * sizeof(sf::Uint8);
            cudaMalloc(&d_pixels, size);
            test_implementation(d_pixels, mandelbrot_cuda_no_cpy, "cuda no cpy", nTests);
            cudaMemcpy(pixels, d_pixels, size, cudaMemcpyDeviceToHost);
            cudaFree(d_pixels);
        }
        else if (strcmp(argv[1], "cuda"       ) == 0) test_implementation(pixels, mandelbrot_cuda,        "cuda",        nTests);
        else
#endif
        if      (strcmp(argv[1], "naive"      ) == 0) test_implementation(pixels, mandelbrot_naive,       "naive",       nTests);
        else if (strcmp(argv[1], "vectorized" ) == 0) test_implementation(pixels, mandelbrot_vectorized,  "vectorized",  nTests);
        else if (strcmp(argv[1], "arrayed"    ) == 0) test_implementation(pixels, mandelbrot_arrayed,     "arrayed",     nTests);
        else if (strcmp(argv[1], "openmp"     ) == 0) test_implementation(pixels, mandelbrot_openmp,      "openmp",      nTests);
        else if (strcmp(argv[1], "thread-pool") == 0) test_implementation(pixels, mandelbrot_thread_pool, "thread-pool", nTests);
        else assert(0 && "Unknown implementation");

        return 0;
    }

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Mandelbrot");

    sf::Texture texture;
    texture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    texture.update(pixels);

    sf::Sprite sprite(texture);

    double scale  = 1.0f;
    double shiftX = 0.0f;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::RBracket)
                {
                    scale *= 1.1;
                }
                else if (event.key.code == sf::Keyboard::LBracket)
                {
                    scale *= 0.9;
                }
                else if (event.key.code == sf::Keyboard::Left)
                {
                    shiftX -= 0.1 / scale;
                }
                else if (event.key.code == sf::Keyboard::Right)
                {
                    shiftX += 0.1 / scale;
                }
            }    
        }

#ifdef GPU
        if (strcmp(argv[1], "cuda_no_cpy") == 0) {
            sf::Uint8* d_pixels;
            size_t size = WINDOW_WIDTH * WINDOW_HEIGHT * 4 * sizeof(sf::Uint8);
            cudaMalloc(&d_pixels, size);
            mandelbrot_cuda_no_cpy(d_pixels, scale, shiftX);
            cudaMemcpy(pixels, d_pixels, size, cudaMemcpyDeviceToHost);
            cudaFree(d_pixels);
        }
        else if (strcmp(argv[1], "cuda"       ) == 0) mandelbrot_cuda(pixels, scale, shiftX);
        else
#endif
        if      (strcmp(argv[1], "naive"      ) == 0) mandelbrot_naive(pixels, scale, shiftX);
        else if (strcmp(argv[1], "vectorized" ) == 0) mandelbrot_vectorized(pixels, scale, shiftX);
        else if (strcmp(argv[1], "arrayed"    ) == 0) mandelbrot_arrayed(pixels, scale, shiftX);
        else if (strcmp(argv[1], "openmp"     ) == 0) mandelbrot_openmp(pixels, scale, shiftX);
        else if (strcmp(argv[1], "thread-pool") == 0) mandelbrot_thread_pool(pixels, scale, shiftX);
        else assert(0 && "Unknown implementation");

        texture.update(pixels);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    free(pixels);

    return 0;
}
