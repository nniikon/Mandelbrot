#ifndef MANDELBROT_OPENMP_H_
#define MANDELBROT_OPENMP_H_

#include <SFML/Graphics.hpp>

void mandelbrot_openmp(sf::Uint8* pixels, float magnifier, float shiftX);

#endif // MANDELBROT_OPENMP_H_
