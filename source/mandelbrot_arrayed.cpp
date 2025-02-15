#include "mandelbrot_arrayed.h"
#include "mandelbrot_config.h"

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
    shiftX    += SHIFT_X_OFFSET;
    magnifier += MAGNIFIER_OFFSET;

    const float invMagnifier = 1.0f / magnifier;
    const float COLOR_SCALE = 255.0f / MAX_ITERATION_DEPTH;

    const float c_step_x = ASPECT_RATIO * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float c_step_y = invMagnifier * (2.0f / WINDOW_HEIGHT);

    const float vec_c_step_x = c_step_x * VEC_SIZE;

    float c_y = -1.0f * invMagnifier + c_step_y * y_from;

    for (int screenY = y_from; screenY < y_to; screenY++) 
    {
        ALIGN float _c_x[VEC_SIZE];

        const float c_x = shiftX - ASPECT_RATIO * invMagnifier;
        FOR_VEC _c_x[i] = c_x + c_step_x * i;

        for (int screenX = 0; screenX < WINDOW_WIDTH; screenX += VEC_SIZE) 
        {
            ALIGN float _z_x [VEC_SIZE] = {0};
            ALIGN float _z_y [VEC_SIZE] = {0};
            ALIGN float _z_xy[VEC_SIZE] = {0};
            ALIGN float _z_x2[VEC_SIZE] = {0};
            ALIGN float _z_y2[VEC_SIZE] = {0};
            ALIGN int _iterations[VEC_SIZE] = {0};

            for (int it = 0; it < MAX_ITERATION_DEPTH; ++it) 
            {
                ALIGN float _radius2[VEC_SIZE] = {0};
                FOR_VEC _radius2[i] = _z_x2[i] + _z_y2[i];

                ALIGN int _is_active[VEC_SIZE] = {0};
                FOR_VEC _is_active[i] = _radius2[i] < MAX_RADIUS_2;

                bool all_zero = true;
                FOR_VEC
                {
                    if (_is_active[i] != 0)
                    {
                        all_zero = false;
                        break;
                    }
                }
                if (all_zero)
                {
                    break;
                }

                FOR_VEC _z_x[i] = _z_x2[i] - _z_y2[i] + _c_x[i];
                FOR_VEC _z_y[i] = 2 * _z_xy[i]     + c_y;

                FOR_VEC _iterations[i] += _is_active[i];

                FOR_VEC _z_x2[i] = _z_x[i] * _z_x[i];
                FOR_VEC _z_y2[i] = _z_y[i] * _z_y[i];
                FOR_VEC _z_xy[i] = _z_x[i] * _z_y[i];
            }

            FOR_VEC
            {
                sf::Uint8 r = 0;
                sf::Uint8 g = 0;
                sf::Uint8 b = 0;
                if (_iterations[i] < MAX_ITERATION_DEPTH)
                {
                    float iterNormalized = _iterations[i] * COLOR_SCALE;
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

            FOR_VEC _c_x[i] += vec_c_step_x;
        }
        c_y += c_step_y;
    }
}
