#include "mandelbrot_arrayed.h"
#include "mandelbrot_config.h"
#include "mandelbrot_color.h"

#include <cstring>

const int VEC_SIZE = 8;

#define FOR_VEC for (int i = 0; i < VEC_SIZE; ++i)
#define ALIGN alignas(VEC_SIZE * 4)

void mandelbrot_arrayed(sf::Uint8* pixels, float magnifier, float shiftX)
{
    return mandelbrot_arrayed_ranged(pixels, magnifier, shiftX, 0, WINDOW_HEIGHT);
}

void mandelbrot_arrayed_ranged(sf::Uint8* pixels, float magnifier, float shiftX,
                               int y_from, int y_to)
{
    const float MAX_RADIUS_2 = MAX_RADIUS * MAX_RADIUS;
    const float scale = (float)(WINDOW_WIDTH) / WINDOW_HEIGHT;

    shiftX -= 0.5f;
    magnifier -= 0.3f;

    const float invMagnifier = 1.0f / magnifier;
    const float colorScale = 255.0f / MAX_ITERATION_DEPTH;

    const float pixel_step_x = scale * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float pixel_step_y = invMagnifier * (2.0f / WINDOW_HEIGHT);

    const float vec_pixel_step_x = pixel_step_x * VEC_SIZE;

    float current_y = -1.0f * invMagnifier + pixel_step_y * y_from;

    for (int screenY = y_from; screenY < y_to; screenY++) 
    {
        ALIGN float current_x[VEC_SIZE];

        // Initialize x coordinates
        const float base_cx = shiftX - scale * invMagnifier;
        FOR_VEC current_x[i] = base_cx + pixel_step_x * i;

        // Process VEC_SIZE pixels at a time
        for (int screenX = 0; screenX < WINDOW_WIDTH; screenX += VEC_SIZE) 
        {
            ALIGN float x [VEC_SIZE] = {0};
            ALIGN float y [VEC_SIZE] = {0};
            ALIGN float xy[VEC_SIZE] = {0};
            ALIGN float x2[VEC_SIZE] = {0};
            ALIGN float y2[VEC_SIZE] = {0};
            ALIGN int iterations[VEC_SIZE] = {0};

            // Iteration loop
            for (int it = 0; it < MAX_ITERATION_DEPTH; ++it) 
            {
                ALIGN float radius2[VEC_SIZE] = {0};
                FOR_VEC radius2[i] = x2[i] + y2[i];

                ALIGN int is_active[VEC_SIZE] = {0};
                FOR_VEC is_active[i] = radius2[i] < MAX_RADIUS_2;

                bool all_zero = true;
                for (int i = 0; i < VEC_SIZE; i++) {
                    if (is_active[i] != 0) {
                        all_zero = false;
                        break;
                    }
                }
                if (all_zero) {
                    break;
                }

                FOR_VEC x[i] = x2[i] - y2[i] + current_x[i];
                FOR_VEC y[i] = 2 * xy[i]     + current_y;

                FOR_VEC iterations[i] += is_active[i];

                FOR_VEC x2[i] = x[i] * x[i];
                FOR_VEC y2[i] = y[i] * y[i];
                FOR_VEC xy[i] = x[i] * y[i];
            }

            for (int i = 0; i < VEC_SIZE; i++)
            {
                sf::Uint8 r = 0;
                sf::Uint8 g = 0;
                sf::Uint8 b = 0;
                if (iterations[i] < MAX_ITERATION_DEPTH)
                {
                    float iterNormalized = iterations[i] * colorScale;
                    r = (sf::Uint8)(iterNormalized / 2);
                    g = (sf::Uint8)(iterNormalized * 2 + 2);
                    b = (sf::Uint8)(iterNormalized * 2 + 5);
                }

                int pixelIndex = (screenY * WINDOW_WIDTH + screenX + i) * 4;
                pixels[pixelIndex + 0] = r;
                pixels[pixelIndex + 1] = g;
                pixels[pixelIndex + 2] = b;
                pixels[pixelIndex + 3] = 255;
            }

            FOR_VEC current_x[i] += vec_pixel_step_x;
        }
        current_y += pixel_step_y;
    }
}
