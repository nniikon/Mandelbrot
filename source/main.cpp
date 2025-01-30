#include <SFML/Graphics.hpp>

#include <cmath>
#include <immintrin.h>

#include "mandelbrot_naive.h"
#include "mandelbrot_optimized.h"
#include "mandelbrot_vectorized.h"
#include "mandelbrot_arrayed.h"
#include "mandelbrot_thread_pool.h"

#include "mandelbrot_config.h"

static void test_implementation(sf::Uint8* pixels, void (*func)(sf::Uint8*, float, float),
                                const char* name, int nTests)
{
    printf("Testing %-20s: ", name);
    uint64_t startTime = __rdtsc();

    for (int i = 0; i < nTests; i++)
        func(pixels, 1.0f, 0.0f);

    uint64_t endTime = __rdtsc();

    float pow = 8.f;
    printf("%5lu * 10^%g ticks\n", (endTime - startTime) / (int)powf(10, pow), pow);
}

void test(sf::Uint8* pixels)
{
    const int nTests = 50;
    test_implementation(pixels, mandelbrot_naive,       "naive"     , nTests);
    test_implementation(pixels, mandelbrot_optimized,   "optimized" , nTests);
    test_implementation(pixels, mandelbrot_arrayed,     "arrayed"   , nTests);
    test_implementation(pixels, mandelbrot_vectorized,  "vectorized", nTests);
    test_implementation(pixels, mandelbrot_thread_pool, "pool"      , nTests);
}

int main()
{
    sf::Uint8* pixels = (sf::Uint8*) calloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4,
                                                            sizeof(sf::Uint8));
    // test(pixels);

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Mondelbrot");

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

        mandelbrot_optimized(pixels, scale, shiftX);
        texture.update(pixels);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    free(pixels);

    return 0;
}
