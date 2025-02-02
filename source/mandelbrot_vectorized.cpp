#include "mandelbrot_color.h"
#include "mandelbrot_vectorized.h"
#include "mandelbrot_config.h"

#include <x86intrin.h>

void mandelbrot_vectorized(sf::Uint8* pixels, float magnifier, float shiftX)
{
    mandelbrot_vectorized_ranged(pixels, magnifier, shiftX, 0, WINDOW_HEIGHT);
}

void mandelbrot_vectorized_ranged(sf::Uint8* pixels, float magnifier, float shiftX,
                                  int y_from, int y_to)
{
    const float MAX_RADUIS_2 = MAX_RADIUS * MAX_RADIUS;
    const float scale = (float)WINDOW_WIDTH / WINDOW_HEIGHT;

    shiftX -= 0.5f;
    magnifier -= 0.3;

    const float invMagnifier = 1.0f / magnifier;
    const float colorScale = 255.0f / MAX_ITERATION_DEPTH;

    const float c_step_x = scale * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float c_step_y = invMagnifier * (2.0f / WINDOW_HEIGHT);

    __m256 _01234567 = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f,
                                     3.0f, 2.0f, 1.0f, 0.0f);
    __m256 _8_c_step_x = _mm256_set1_ps(c_step_x * 8);
    __m256   _c_step_x = _mm256_set1_ps(c_step_x);
    __m256   _c_step_y = _mm256_set1_ps(c_step_y);

    __m256 _maxRadius2 = _mm256_set1_ps(MAX_RADUIS_2);

    float c_y = -1.0f * invMagnifier + c_step_y * y_from;
    __m256 _c_y = _mm256_set1_ps(c_y);
    for (int screenY = y_from; screenY < y_to; ++screenY, c_y += c_step_y)
    {
        float c_x = shiftX - scale * invMagnifier;
        __m256 _c_x = _mm256_set1_ps(c_x);

        // _cx + ( 7dx, 6dx, 5dx, 4dx, 3dx, 2dx, 1dx, 0 )
        _c_x = _mm256_add_ps(_c_x, _mm256_mul_ps(_c_step_x, _01234567));
        for (int screenX = 0; screenX < WINDOW_WIDTH; screenX += 8)
        {
            __m256 _z_x = _mm256_setzero_ps();
            __m256 _z_y = _mm256_setzero_ps();

            __m256 _z_x2 = _mm256_setzero_ps();
            __m256 _z_y2 = _mm256_setzero_ps();
            __m256 _z_xy = _mm256_setzero_ps();

            __m256i _iterations = _mm256_setzero_si256();

            for (int iteration = 0; iteration < MAX_ITERATION_DEPTH; iteration++)
            {
                __m256 _radius2 = _mm256_add_ps(_z_x2, _z_y2);

                __m256 _cmpMask = _mm256_cmp_ps(_radius2, _maxRadius2, _CMP_LT_OQ);
                int mask = _mm256_movemask_ps(_cmpMask);
                // for each float:
                //      if (radius^2 < maxRadius^2)
                //          mask = -1
                //      else
                //          mask = 0

                if (!mask)
                    break;

                // x = x^2 - y^2 + cx
                // y = 2xy + cy
                _z_x = _mm256_add_ps(_c_x, _mm256_sub_ps(_z_x2, _z_y2));
                _z_y = _mm256_add_ps(_c_y, _mm256_mul_ps(_mm256_set1_ps(2.0f), _z_xy));

                // if (radius^2 < maxRadiud^2) iteration++
                _iterations = _mm256_sub_epi32(_iterations, _mm256_castps_si256(_cmpMask));

                _z_x2 = _mm256_mul_ps(_z_x, _z_x);
                _z_y2 = _mm256_mul_ps(_z_y, _z_y);
                _z_xy = _mm256_mul_ps(_z_x, _z_y);
            }

            int iterationsArray[8] = {};
            _mm256_storeu_si256((__m256i*)iterationsArray, _iterations);
            for (int i = 0; i < 8; i++)
            {
                sf::Uint8 r = 0;
                sf::Uint8 g = 0;
                sf::Uint8 b = 0;
                if (iterationsArray[i] < MAX_ITERATION_DEPTH)
                {
                    float iterNormalized = iterationsArray[i] * colorScale;
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

            _c_x = _mm256_add_ps(_c_x, _8_c_step_x);
        }
        _c_y = _mm256_add_ps(_c_y, _c_step_y);
    }
}
