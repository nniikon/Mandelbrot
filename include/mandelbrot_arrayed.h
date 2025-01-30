#ifndef MANDELBROT_ARRAYED_H_
#define MANDELBROT_ARRAYED_H_

#include <SFML/Graphics.hpp>

void mandelbrot_arrayed_ranged(sf::Uint8* pixels, float magnifier, float shiftX,
                               int y_from, int y_to);

void mandelbrot_arrayed(sf::Uint8* pixels, float magnifier, float shiftX);

#endif // MANDELBROT_ARRAYED_H_
