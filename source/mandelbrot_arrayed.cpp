#include "mandelbrot_arrayed.h"
#include "mandelbrot_config.h"
#include "mandelbrot_color.h"

#include <cstring>

const int VEC_SIZE = 8;

#define FOR for (int i = 0; i < VEC_SIZE; ++i)
#define ALIGN alignas(VEC_SIZE * sizeof(float))

const int ZERO[VEC_SIZE] = {};

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

    const float pixel_step_x = scale * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float pixel_step_y = invMagnifier * (2.0f / WINDOW_HEIGHT);

    const float vec_pixel_step_x = pixel_step_x * VEC_SIZE;

    float current_y = -1.0f * invMagnifier + pixel_step_y * y_from;

    #pragma omp parallel for
    for (int screenY = y_from; screenY < y_to; screenY++) 
    {
        ALIGN float current_x[VEC_SIZE];

        // Initialize x coordinates
        const float base_cx = shiftX - scale * invMagnifier;
        FOR current_x[i] = base_cx + pixel_step_x * i;

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
                FOR radius2[i] = x2[i] + y2[i];

                ALIGN int is_active[VEC_SIZE] = {0};
                FOR is_active[i] = radius2[i] < MAX_RADIUS_2;

                if (memcmp(is_active, ZERO, VEC_SIZE * sizeof(int)) == 0) break;

                FOR x[i] = x2[i] - y2[i] + current_x[i];
                FOR y[i] = 2 * xy[i]     + current_y;

                FOR iterations[i] += is_active[i];

                FOR x2[i] = x[i] * x[i];
                FOR y2[i] = y[i] * y[i];
                FOR xy[i] = x[i] * y[i];
            }

            #pragma omp simd
            for (int i = 0; i < VEC_SIZE; ++i) 
            {
                const int pixelIndex = (screenY * WINDOW_WIDTH + screenX + i) * 4;

                sf::Color color = GetColor(iterations[i], MAX_ITERATION_DEPTH);

                pixels[pixelIndex + 0] = color.r;
                pixels[pixelIndex + 1] = color.g;
                pixels[pixelIndex + 2] = color.b;
                pixels[pixelIndex + 3] = 255;
            }

            FOR current_x[i] += vec_pixel_step_x;
        }
        current_y += pixel_step_y;
    }
}

#if SIMPLIFIED

void mandelbrot_arrayed(sf::Uint8* pixels, float magnifier, float shiftX)
{
    const float MAX_RADIUS_2 = MAX_RADIUS * MAX_RADIUS;

    const float pixel_step_x = 2.0f / WINDOW_WIDTH;
    const float pixel_step_y = 2.0f / WINDOW_HEIGHT;

    const float vec_pixel_step_x = pixel_step_x * VEC_SIZE;

    float current_y = -1.0f;

    #pragma omp parallel for
    for (int screenY = 0; screenY < WINDOW_HEIGHT; screenY++) 
    {
        ALIGN float current_x[VEC_SIZE];

        // Initialize x coordinates
        const float base_cx = -1.0f;
        FOR current_x[i] = base_cx + pixel_step_x * i;

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
                FOR radius2[i] = x2[i] + y2[i];

                ALIGN int is_active[VEC_SIZE] = {0};
                FOR is_active[i] = radius2[i] < MAX_RADIUS_2;

                if (memcmp(is_active, ZERO, VEC_SIZE * sizeof(int)) == 0) break;

                FOR x[i] = x2[i] - y2[i] + current_x[i];
                FOR y[i] = 2 * xy[i]     + current_y;

                FOR iterations[i] += is_active[i];

                FOR x2[i] = x[i] * x[i];
                FOR y2[i] = y[i] * y[i];
                FOR xy[i] = x[i] * y[i];
            }

            #pragma omp simd
            FOR
            {
                const int pixelIndex = (screenY * WINDOW_WIDTH + screenX + i) * 4;

                sf::Color color = GetColor(iterations[i], MAX_ITERATION_DEPTH);

                pixels[pixelIndex + 0] = color.r;
                pixels[pixelIndex + 1] = color.g;
                pixels[pixelIndex + 2] = color.b;
                pixels[pixelIndex + 3] = 255;
            }

            FOR current_x[i] += vec_pixel_step_x;
        }

        current_y += pixel_step_y;
    }
}

#endif
