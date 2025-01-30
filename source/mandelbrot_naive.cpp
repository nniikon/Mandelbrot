#include "mandelbrot_naive.h"
#include "mandelbrot_config.h"
#include "mandelbrot_color.h"

void mandelbrot_naive(sf::Uint8* pixels, float magnifier, float shiftX) 
{
    // centering parameters
    shiftX -= 0.5f;
    magnifier -= 0.3f;

    const float MAX_RADIUS_2 = MAX_RADIUS * MAX_RADIUS;
    const float ASPECT_RATIO = (float)(WINDOW_WIDTH) / WINDOW_HEIGHT;

    const float inv_magnifier = 1.0f / magnifier;

    const float pixel_step_x = ASPECT_RATIO * inv_magnifier * (2.0f / WINDOW_WIDTH);
    const float pixel_step_y = inv_magnifier                * (2.0f / WINDOW_HEIGHT);

    float current_y_coord = -1.0f * inv_magnifier;

    for (int screen_y = 0; screen_y < WINDOW_HEIGHT; screen_y++,
                                                     current_y_coord += pixel_step_y) 
    {
        float current_x_coord = shiftX - ASPECT_RATIO * inv_magnifier;

        for (int screen_x = 0; screen_x < WINDOW_WIDTH; ++screen_x, 
                                                        current_x_coord += pixel_step_x) 
        {
            int iterations = 0;

            float z_x  = 0.0f;
            float z_y  = 0.0f;
            float z_x2 = 0.0f;
            float z_y2 = 0.0f;

            while (z_x2 + z_y2 < MAX_RADIUS_2 &&
                   iterations < MAX_ITERATION_DEPTH) 
            {
                z_y = 2 * z_x * z_y + current_y_coord;
                z_x = z_x2 - z_y2 + current_x_coord;

                z_x2 = z_x * z_x;
                z_y2 = z_y * z_y;

                ++iterations;
            }

            sf::Color pixel_color = GetColor(iterations, MAX_ITERATION_DEPTH);
            const int pixel_index = (screen_y * WINDOW_WIDTH + screen_x) * 4;
            
            pixels[pixel_index + 0] = pixel_color.r;
            pixels[pixel_index + 1] = pixel_color.g;
            pixels[pixel_index + 2] = pixel_color.b;
            pixels[pixel_index + 3] = 255u;
        }
    }
}

#if SIMPLIFIED

void mandelbrot_naive(sf::Uint8* pixels, float magnifier, float shiftX) 
{
    const float MAX_RADIUS_2 = MAX_RADIUS * MAX_RADIUS;

    const float pixel_step_x = (2.0f / WINDOW_WIDTH);
    const float pixel_step_y = (2.0f / WINDOW_HEIGHT);

    float current_y_coord = -1.0f;

    for (int screen_y = 0; screen_y < WINDOW_HEIGHT; screen_y++,
                                                     current_y_coord += pixel_step_y) 
    {
        float current_x_coord = -1.0f;

        for (int screen_x = 0; screen_x < WINDOW_WIDTH; ++screen_x, 
                                                        current_x_coord += pixel_step_x) 
        {
            int iterations = 0;

            float z_x  = 0.0f;
            float z_y  = 0.0f;
            float z_x2 = 0.0f;
            float z_y2 = 0.0f;

            while (z_x2 + z_y2 < MAX_RADIUS_2 &&
                   iterations < MAX_ITERATION_DEPTH) 
            {
                z_y = 2 * z_x * z_y + current_y_coord;
                z_x = z_x2 - z_y2 + current_x_coord;

                z_x2 = z_x * z_x;
                z_y2 = z_y * z_y;

                ++iterations;
            }

            sf::Color pixel_color = GetColor(iterations, MAX_ITERATION_DEPTH);
            const int pixel_index = (screen_y * WINDOW_WIDTH + screen_x) * 4;
            
            pixels[pixel_index + 0] = pixel_color.r;
            pixels[pixel_index + 1] = pixel_color.g;
            pixels[pixel_index + 2] = pixel_color.b;
            pixels[pixel_index + 3] = 255u;
        }
    }
}

#endif
