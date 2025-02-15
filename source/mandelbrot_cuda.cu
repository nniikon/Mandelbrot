#include "mandelbrot_config.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <assert.h>
#include <SFML/Graphics.hpp>

__global__ void mandelbrot_kernel(sf::Uint8* pixels, float magnifier, float shiftX)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= WINDOW_WIDTH || y >= WINDOW_HEIGHT)
        return;
    
    const float inv_magnifier = 1.0f / magnifier;
    const float COLOR_SCALE = 255.0f / MAX_ITERATION_DEPTH;

    float pixel_step_x = ASPECT_RATIO * inv_magnifier * (2.0f / WINDOW_WIDTH);
    float pixel_step_y = inv_magnifier * (2.0f / WINDOW_HEIGHT);

    float current_x_coord = shiftX - ASPECT_RATIO * inv_magnifier + x * pixel_step_x;
    float current_y_coord = -1.0f * inv_magnifier + y * pixel_step_y;

    int iterations = 0;
    float z_x = 0.0f, z_y = 0.0f, z_x2 = 0.0f, z_y2 = 0.0f;

    while (z_x2 + z_y2 < MAX_RADIUS_2 && iterations < MAX_ITERATION_DEPTH) {
        z_y = 2 * z_x * z_y + current_y_coord;
        z_x = z_x2 - z_y2 + current_x_coord;
        z_x2 = z_x * z_x;
        z_y2 = z_y * z_y;
        iterations++;
    }

    sf::Uint8 r = 0, g = 0, b = 0;
    if (iterations < MAX_ITERATION_DEPTH) {
        float iterNormalized = iterations * COLOR_SCALE;
        r = (sf::Uint8)(iterNormalized / 2);
        g = (sf::Uint8)(iterNormalized * 2 + 2);
        b = (sf::Uint8)(iterNormalized * 2 + 5);
    }

    int pixelIndex = (y * WINDOW_WIDTH + x) * 4;
    pixels[pixelIndex + 0] = r;
    pixels[pixelIndex + 1] = g;
    pixels[pixelIndex + 2] = b;
    pixels[pixelIndex + 3] = 255;
}

void mandelbrot_cuda(sf::Uint8* pixels, float magnifier, float shiftX) {
    shiftX    += SHIFT_X_OFFSET;
    magnifier += MAGNIFIER_OFFSET;
    
    sf::Uint8* d_pixels;
    size_t size = WINDOW_WIDTH * WINDOW_HEIGHT * 4 * sizeof(sf::Uint8);
    cudaMalloc(&d_pixels, size);
    
    dim3 blockSize(16, 16);
    dim3 gridSize((WINDOW_WIDTH + blockSize.x - 1) / blockSize.x, (WINDOW_HEIGHT + blockSize.y - 1) / blockSize.y);
    
    mandelbrot_kernel<<<gridSize, blockSize>>>(d_pixels, magnifier, shiftX);
    cudaMemcpy(pixels, d_pixels, size, cudaMemcpyDeviceToHost);
    cudaFree(d_pixels);
}

void mandelbrot_cuda_no_cpy(sf::Uint8* pixels, float magnifier, float shiftX) {
    shiftX -= 0.5f;
    magnifier -= 0.3f;

    dim3 blockSize(16, 16);
    dim3 gridSize((WINDOW_WIDTH  + blockSize.x - 1) / blockSize.x,
                  (WINDOW_HEIGHT + blockSize.y - 1) / blockSize.y);
    
    mandelbrot_kernel<<<gridSize, blockSize>>>(pixels, magnifier, shiftX);
}
