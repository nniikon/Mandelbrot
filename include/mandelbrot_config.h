#ifndef MANDELBROT_CONFIG_H_
#define MANDELBROT_CONFIG_H_

const int WINDOW_WIDTH  = 1920;
const int WINDOW_HEIGHT = 1080;
const int N_THREADS = 12;

const int   MAX_ITERATION_DEPTH = 256;
const float MAX_RADIUS          = 2.0f;

const float MAX_RADIUS_2 = MAX_RADIUS * MAX_RADIUS;
const float ASPECT_RATIO = (float)(WINDOW_WIDTH) / WINDOW_HEIGHT;

const float MAGNIFIER_OFFSET = -0.3f;
const float SHIFT_X_OFFSET   = -0.5f;

#endif // MANDELBROT_CONFIG_H_
