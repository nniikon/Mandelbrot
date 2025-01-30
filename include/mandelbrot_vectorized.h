#ifndef MANDELBROT_VECTORIZED_H_
#define MANDELBROT_VECTORIZED_H_

#include <SFML/Graphics.hpp>

void mandelbrot_vectorized_internal(sf::Uint8* pixels, float magnifier, float shiftX,
                                    int leftBound, int rightBound);

void mandelbrot_vectorized(sf::Uint8* pixels, float magnifier, float shiftX);

#endif // MANDELBROT_VECTORIZED_H_
