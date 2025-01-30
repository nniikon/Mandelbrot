#include "mandelbrot_naive.h"
#include "mandelbrot_config.h"

void mandelbrot_optimized(sf::Uint8* pixels, float magnifier, float shiftX)
{
    const float radius2 = MAX_RADIUS * MAX_RADIUS;
    const float scale = (float)WINDOW_WIDTH / WINDOW_HEIGHT;

    shiftX -= 0.5f;
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

            sf::Uint8 r = 0;
            sf::Uint8 g = 0;
            sf::Uint8 b = 0;
            if (iteration < MAX_ITERATION_DEPTH)
            {
                float iterNormalized = iteration * colorScale;
                r = (sf::Uint8)(iterNormalized / 2);
                g = (sf::Uint8)(iterNormalized * 2 + 2);
                b = (sf::Uint8)(iterNormalized * 2 + 5);
            }

            int pixelIndex = (screenY * WINDOW_WIDTH + screenX) * 4;
            pixels[pixelIndex + 0] = r;
            pixels[pixelIndex + 1] = g;
            pixels[pixelIndex + 2] = b;
            pixels[pixelIndex + 3] = 255;
        }
    }
}
