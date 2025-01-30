#include "mandelbrot_vectorized.h"
#include "mandelbrot_config.h"

#include <x86intrin.h>

void mandelbrot_vectorized_internal(sf::Uint8* pixels, float magnifier, float shiftX,
                                    int leftBound, int rightBound)
{
    const float radius2 = MAX_RADIUS * MAX_RADIUS;
    const float scale = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    shiftX -= 0.5f;
    magnifier -= 0.3;

    const float invMagnifier = 1.0f / magnifier;
    const float colorScale = 255.0f / MAX_ITERATION_DEPTH;

    const float dx = scale * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float dy = invMagnifier * (2.0f / WINDOW_HEIGHT);

    __m256 _01234567 = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f,
                                     3.0f, 2.0f, 1.0f, 0.0f);
    __m256 _8dx = _mm256_set1_ps(dx * 8);
    __m256  _dx = _mm256_set1_ps(dx);
    __m256  _dy = _mm256_set1_ps(dy);

    __m256 _maxRadius2 = _mm256_set1_ps(radius2);

    float cy = -1.0f * invMagnifier + dy * leftBound;
    __m256 _cy = _mm256_set1_ps(cy);
    for (int screenY = leftBound; screenY < rightBound; ++screenY, cy += dy)
    {
        float cx = shiftX - scale * invMagnifier;
        __m256 _cx = _mm256_set1_ps(cx);

        // _cx + ( 7dx, 6dx, 5dx, 4dx, 3dx, 2dx, 1dx, 0 )
        _cx = _mm256_add_ps(_cx, _mm256_mul_ps(_dx, _01234567));
        for (int screenX = 0; screenX < WINDOW_WIDTH; screenX += 8)
        {
            __m256 _x = _mm256_setzero_ps();
            __m256 _y = _mm256_setzero_ps();

            __m256 _x2 = _mm256_setzero_ps();
            __m256 _y2 = _mm256_setzero_ps();
            __m256 _xy = _mm256_setzero_ps();

            __m256i _iterations = _mm256_setzero_si256();

            for (int iteration = 0; iteration < MAX_ITERATION_DEPTH; iteration++)
            {
                __m256 _radius2 = _mm256_add_ps(_x2, _y2);

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
                _x = _mm256_add_ps(_cx, _mm256_sub_ps(_x2, _y2));
                _y = _mm256_add_ps(_cy, _mm256_mul_ps(_mm256_set1_ps(2.0f), _xy));

                // if (radius^2 < maxRadiud^2) iteration++
                _iterations = _mm256_sub_epi32(_iterations, _mm256_castps_si256(_cmpMask));

                _x2 = _mm256_mul_ps(_x, _x);
                _y2 = _mm256_mul_ps(_y, _y);
                _xy = _mm256_mul_ps(_x, _y);
            }

            int* iterationsArray = (int*)&_iterations;
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

            _cx = _mm256_add_ps(_cx, _8dx);
        }
        _cy = _mm256_add_ps(_cy, _dy);
    }
}

void mandelbrot_vectorized(sf::Uint8* pixels, float magnifier, float shiftX)
{
    mandelbrot_vectorized_internal(pixels, magnifier, shiftX, 0.0f, WINDOW_HEIGHT);
}
