#include "mandelbrot_naive.h"
#include "mandelbrot_config.h"

void mandelbrot_naive(sf::Uint8* pixels, float magnifier, float shiftX) 
{
    shiftX    += SHIFT_X_OFFSET;
    magnifier += MAGNIFIER_OFFSET;

    const float inv_magnifier = 1.0f / magnifier;
    const float COLOR_SCALE = 255.0f / MAX_ITERATION_DEPTH;

    const float pixel_step_x = ASPECT_RATIO * inv_magnifier * (2.0f / WINDOW_WIDTH);
    const float pixel_step_y = inv_magnifier                * (2.0f / WINDOW_HEIGHT);

    float c_y = -1.0f * inv_magnifier;

    for (int screen_y = 0; screen_y < WINDOW_HEIGHT; screen_y++,
                                                     c_y += pixel_step_y) 
    {
        float c_x = shiftX - ASPECT_RATIO * inv_magnifier;

        for (int screen_x = 0; screen_x < WINDOW_WIDTH; screen_x++, 
                                                        c_x += pixel_step_x) 
        {
            int iterations = 0;

            float z_x  = 0.0f;
            float z_y  = 0.0f;
            float z_x2 = 0.0f;
            float z_y2 = 0.0f;

            while (z_x2 + z_y2 < MAX_RADIUS_2 &&
                   iterations < MAX_ITERATION_DEPTH) 
            {
                z_y = 2 * z_x * z_y + c_y;
                z_x = z_x2 - z_y2 + c_x;

                z_x2 = z_x * z_x;
                z_y2 = z_y * z_y;

                ++iterations;
            }

                sf::Uint8 r = 0;
                sf::Uint8 g = 0;
                sf::Uint8 b = 0;
                if (iterations < MAX_ITERATION_DEPTH)
                {
                    float iterNormalized = iterations * COLOR_SCALE;
                    r = (sf::Uint8)(iterNormalized / 2);
                    g = (sf::Uint8)(iterNormalized * 2 + 2);
                    b = (sf::Uint8)(iterNormalized * 2 + 5);
                }

                int pixelIndex = (screen_y * WINDOW_WIDTH + screen_x) * 4;
                pixels[pixelIndex + 0] = r;
                pixels[pixelIndex + 1] = g;
                pixels[pixelIndex + 2] = b;
                pixels[pixelIndex + 3] = 255;
        }
    }
}
