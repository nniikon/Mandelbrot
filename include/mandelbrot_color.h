#ifndef MANDELBROT_COLOR_H_
#define MANDELBROT_COLOR_H_

#include <SFML/Graphics.hpp>

__always_inline sf::Color GetColor(int iteration, int maxIterations)
{
    if (iteration >= maxIterations) {
        return sf::Color::Black;
    }

    const float colorScale = 255.0f / maxIterations;
    const float iterNormalized = iteration * colorScale;

    const sf::Uint8 r = (sf::Uint8)(iterNormalized / 2 + 0);
    const sf::Uint8 g = (sf::Uint8)(iterNormalized * 2 + 2);
    const sf::Uint8 b = (sf::Uint8)(iterNormalized * 2 + 5);

    return sf::Color(r, g, b, 255);
}

#endif // MANDELBROT_COLOR_H_
