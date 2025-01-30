#include "mandelbrot_thread_pool.h"
#include "mandelbrot_vectorized.h"
#include "mandelbrot_config.h"

#include "thread_pool.h"

TH_Pool pool;

struct MandelbrotArgs
{
    sf::Uint8* pixels;
    float magnifier;
    float shiftX;
    int leftBound;
    int rightBound;
};

void mandelbrot_pool_internal(void* mandelStruct)
{
    MandelbrotArgs* args = (MandelbrotArgs*) mandelStruct;

    mandelbrot_vectorized_internal(args->pixels, args->magnifier, 
                                   args->shiftX, args->leftBound, args->rightBound);
}

void mandelbrot_thread_pool(sf::Uint8* pixels, float magnifier, float shiftX)
{
    // FIXME: causes a memleak
    static bool isPoolInitialized = false;
    if (!isPoolInitialized)
    {
        TH_PoolInit(&pool, N_THREADS);
        isPoolInitialized = true;
    }

    int step = 8;

    MandelbrotArgs args[WINDOW_HEIGHT / 8] = {};
    for (int i = 0; i < WINDOW_HEIGHT; i += step)
    {
        args[i/8] = 
        {
            .pixels = pixels,
            .magnifier = magnifier,
            .shiftX = shiftX,
            .leftBound = i,
            .rightBound = i + step,
        };
        TH_Job job = 
        {
            .func = (void (*)(void*)) mandelbrot_pool_internal,
            .args = &args[i/8],
        };
        TH_PoolAddJob(&pool, job);
    }

    TH_PoolWait(&pool);
    
    return;
}
