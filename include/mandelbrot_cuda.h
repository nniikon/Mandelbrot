#ifndef MANDELBROT_CUDA_H_
#define MANDELBROT_CUDA_H_

#include <SFML/Graphics.hpp>

void mandelbrot_cuda       (sf::Uint8* pixels, float magnifier, float shiftX);
void mandelbrot_cuda_no_cpy(sf::Uint8* pixels, float magnifier, float shiftX);

#endif // MANDELBROT_CUDA_H_
