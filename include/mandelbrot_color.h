#ifndef MANDELBROT_COLOR_H_
#define MANDELBROT_COLOR_H_

#include <SFML/Graphics.hpp>
#include "mandelbrot_config.h"

inline __attribute__((always_inline)) sf::Color get_color(int iteration, int maxIterations)
{
    if (iteration < MAX_ITERATION_DEPTH) {
        const float colorScale = 255.0f / MAX_ITERATION_DEPTH;
        const float iterNormalized = iteration * colorScale;

        const sf::Uint8 r = (sf::Uint8)(iterNormalized / 2 + 0);
        const sf::Uint8 g = (sf::Uint8)(iterNormalized * 2 + 2);
        const sf::Uint8 b = (sf::Uint8)(iterNormalized * 2 + 5);

        return sf::Color(r, g, b, 255);
    }

    return sf::Color::Black;
}

#endif // MANDELBROT_COLOR_H_
