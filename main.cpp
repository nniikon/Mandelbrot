#include <SFML/Graphics.hpp>
#include <immintrin.h>

const int WINDOW_WIDTH  = 1920;
const int WINDOW_HEIGHT = 1080;

const int   MAX_ITERATION_DEPTH = 100;
const float MAX_RADIUS          = 50.0;

void mandelbrot_naive     (sf::Uint8* pixels, double magnifier);

int main() {

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Mondelbrot");

    sf::Uint8* pixels = new sf::Uint8[WINDOW_WIDTH * WINDOW_HEIGHT * 4];

    sf::Texture texture;
    texture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    texture.update(pixels);

    sf::Sprite sprite(texture);

    double scale = 1.0f;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    
        mandelbrot_naive(pixels, scale);
        texture.update(pixels);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    delete[] pixels;

    return 0;
}

void mandelbrot_naive(sf::Uint8* pixels, double magnifier)
{
    const float radius2 = MAX_RADIUS * MAX_RADIUS;
    const float scale = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    const float shiftX = -0.5f;
    magnifier -= 0.3;

    const float invMagnifier = 1.0f / magnifier;
    const float colorScale = 255.0f / MAX_ITERATION_DEPTH;

    const float dx = scale * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float dy = invMagnifier * (2.0f / WINDOW_HEIGHT);

    float cy = -1.0f * invMagnifier;
    for (int screenY = 0; screenY < WINDOW_HEIGHT; ++screenY, cy += dy)
    {
        float cx = shiftX - scale * invMagnifier;
        for (int screenX = 0; screenX < WINDOW_WIDTH; ++screenX, cx += dx)
        {
            float x = 0.0f;
            float y = 0.0f;
            int iteration = 0;

            float x2 = 0.0f;
            float y2 = 0.0f;

            while (x2 + y2 < radius2 && iteration < MAX_ITERATION_DEPTH)
            {
                y = 2 * x * y + cy;
                x = x2 - y2 + cx;

                x2 = x * x;
                y2 = y * y;
                ++iteration;
            }

            sf::Uint8 r, g, b;
            if (iteration < MAX_ITERATION_DEPTH)
            {
                float iterNormalized = iteration * colorScale;
                r = (sf::Uint8)(iterNormalized / 2);
                g = (sf::Uint8)(iterNormalized * 2 + 2);
                b = (sf::Uint8)(iterNormalized * 2 + 5);
            }
            else 
            {
                r = 0;
                g = 0;
                b = 0;
            }

            int pixelIndex = (screenY * WINDOW_WIDTH + screenX) * 4;
            pixels[pixelIndex + 0] = r;
            pixels[pixelIndex + 1] = g;
            pixels[pixelIndex + 2] = b;
            pixels[pixelIndex + 3] = 255;
        }
    }
}


