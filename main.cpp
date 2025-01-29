#include <SFML/Graphics.hpp>
#include <immintrin.h>
#include <thread>
#include <x86intrin.h>

#include "Thread-Pool/thread_pool.h"

const int WINDOW_WIDTH  = 1920;
const int WINDOW_HEIGHT = 1080;
const int N_THREADS = 12;

const int   MAX_ITERATION_DEPTH = 256;
const float MAX_RADIUS          = 50.0;

void mandelbrot_naive        (sf::Uint8* pixels, float magnifier, float shiftX);
void mandelbrot_vectorized   (sf::Uint8* pixels, float magnifier, float shiftX);
void mandelbrot_multithreaded(sf::Uint8* pixels, float magnifier, float shiftX);
void mandelbrot_unfair       (sf::Uint8* pixels, float magnifier, float shiftX);
void mandelbrot_pool         (sf::Uint8* pixels, float magnifier, float shiftX);

TH_Pool pool;

static void test_implem(sf::Uint8* pixels, void (*func)(sf::Uint8*, float, float),
                            const char* name, int nTests)
{
    printf("Testing %-50s: ", name);
    uint64_t sperma1 = __rdtsc();

    for (int i = 0; i < nTests; i++)
        func(pixels, 1.0f, 0.0f);

    uint64_t sperma2 = __rdtsc();
    printf("%lu * 10^8 ticks\n", (sperma2 - sperma1) / 100'000'000);
}

void test(sf::Uint8* pixels)
{
    const int nTests = 2500;
    // test_implem(pixels, mandelbrot_naive,         "naive"     ,    nTests);
    test_implem(pixels, mandelbrot_vectorized,    "vectorized",    nTests);
    test_implem(pixels, mandelbrot_multithreaded, "multithreaded", nTests);
    test_implem(pixels, mandelbrot_unfair,        "unfair",        nTests);
    test_implem(pixels, mandelbrot_pool,          "pool"     ,     nTests);
}

int main()
{
    TH_PoolInit(&pool, N_THREADS);

    sf::Uint8* pixels = (sf::Uint8*) calloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4, sizeof(sf::Uint8));
    test(pixels);

    return 0;

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Mondelbrot");

    sf::Texture texture;
    texture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    texture.update(pixels);

    sf::Sprite sprite(texture);

    double scale  = 1.0f;
    double shiftX = 0.0f;
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::RBracket)
                {
                    scale *= 1.1;
                }
                else if (event.key.code == sf::Keyboard::LBracket)
                {
                    scale *= 0.9;
                }
                else if (event.key.code == sf::Keyboard::Left)
                {
                    shiftX -= 0.1 / scale;
                }
                else if (event.key.code == sf::Keyboard::Right)
                {
                    shiftX += 0.1 / scale;
                }
            }    
        }
        
        mandelbrot_pool(pixels, scale, shiftX);
        texture.update(pixels);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    free(pixels);

    TH_PoolDtor(&pool);

    return 0;
}

void mandelbrot(sf::Uint8* pixels, float magnifier, float shiftX);

void mandelbrot_naive(sf::Uint8* pixels, float magnifier, float shiftX)
{
    const float radius2 = MAX_RADIUS * MAX_RADIUS;
    const float scale = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    shiftX -= 0.5f;
    magnifier -= 0.3;

    const float invMagnifier = 1.0f / magnifier;
    const float colorScale = 255.0f / MAX_ITERATION_DEPTH;

    const float dx = scale * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float dy = invMagnifier * (2.0f / WINDOW_HEIGHT);

    float cy = -1.0f * invMagnifier;
    for (int screenY = 0; screenY < WINDOW_HEIGHT; ++screenY, cy += dy)
    {
        float cx = shiftX - scale * invMagnifier;
        for (int screenX = 0; screenX < WINDOW_WIDTH; ++screenX, cx += dx)
        {
            float x = 0.0f;
            float y = 0.0f;
            int iteration = 0;

            float x2 = 0.0f;
            float y2 = 0.0f;

            while (x2 + y2 < radius2 && iteration < MAX_ITERATION_DEPTH)
            {
                y = 2 * x * y + cy;
                x = x2 - y2 + cx;

                x2 = x * x;
                y2 = y * y;
                ++iteration;
            }

            sf::Uint8 r = 0;
            sf::Uint8 g = 0;
            sf::Uint8 b = 0;
            if (iteration < MAX_ITERATION_DEPTH)
            {
                float iterNormalized = iteration * colorScale;
                r = (sf::Uint8)(iterNormalized / 2);
                g = (sf::Uint8)(iterNormalized * 2 + 2);
                b = (sf::Uint8)(iterNormalized * 2 + 5);
            }

            int pixelIndex = (screenY * WINDOW_WIDTH + screenX) * 4;
            pixels[pixelIndex + 0] = r;
            pixels[pixelIndex + 1] = g;
            pixels[pixelIndex + 2] = b;
            pixels[pixelIndex + 3] = 255;
        }
    }
}


static void mandelbrot_vectorized_internal(sf::Uint8* pixels, float magnifier, float shiftX,
                           int leftBound, int rightBound)
{
    const float radius2 = MAX_RADIUS * MAX_RADIUS;
    const float scale = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    shiftX -= 0.5f;
    magnifier -= 0.3;

    const float invMagnifier = 1.0f / magnifier;
    const float colorScale = 255.0f / MAX_ITERATION_DEPTH;

    const float dx = scale * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float dy = invMagnifier * (2.0f / WINDOW_HEIGHT);

    __m256 _01234567 = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f,
                                     3.0f, 2.0f, 1.0f, 0.0f);
    __m256 _8dx = _mm256_set1_ps(dx * 8);
    __m256  _dx = _mm256_set1_ps(dx);
    __m256  _dy = _mm256_set1_ps(dy);

    __m256 _maxRadius2 = _mm256_set1_ps(radius2);

    float cy = -1.0f * invMagnifier + dy * leftBound;
    __m256 _cy = _mm256_set1_ps(cy);
    for (int screenY = leftBound; screenY < rightBound; ++screenY, cy += dy)
    {
        float cx = shiftX - scale * invMagnifier;
        __m256 _cx = _mm256_set1_ps(cx);

        // _cx + ( 7dx, 6dx, 5dx, 4dx, 3dx, 2dx, 1dx, 0 )
        _cx = _mm256_add_ps(_cx, _mm256_mul_ps(_dx, _01234567));
        for (int screenX = 0; screenX < WINDOW_WIDTH; screenX += 8)
        {
            __m256 _x = _mm256_setzero_ps();
            __m256 _y = _mm256_setzero_ps();

            __m256 _x2 = _mm256_setzero_ps();
            __m256 _y2 = _mm256_setzero_ps();
            __m256 _xy = _mm256_setzero_ps();

            __m256i _iterations = _mm256_setzero_si256();

            for (int iteration = 0; iteration < MAX_ITERATION_DEPTH; iteration++)
            {
                __m256 _radius2 = _mm256_add_ps(_x2, _y2);

                __m256 _cmpMask = _mm256_cmp_ps(_radius2, _maxRadius2, _CMP_LT_OQ);
                int mask = _mm256_movemask_ps(_cmpMask);
                // for each float:
                //      if (radius^2 < maxRadius^2)
                //          mask = -1
                //      else
                //          mask = 0

                if (!mask)
                    break;

                // x = x^2 - y^2 + cx
                // y = 2xy + cy
                _x = _mm256_add_ps(_cx, _mm256_sub_ps(_x2, _y2));
                _y = _mm256_add_ps(_cy, _mm256_mul_ps(_mm256_set1_ps(2.0f), _xy));

                // if (radius^2 < maxRadiud^2) iteration++
                _iterations = _mm256_sub_epi32(_iterations, _mm256_castps_si256(_cmpMask));

                _x2 = _mm256_mul_ps(_x, _x);
                _y2 = _mm256_mul_ps(_y, _y);
                _xy = _mm256_mul_ps(_x, _y);
            }

            int* iterationsArray = (int*)&_iterations;
            for (int i = 0; i < 8; i++)
            {
                sf::Uint8 r = 0;
                sf::Uint8 g = 0;
                sf::Uint8 b = 0;
                if (iterationsArray[i] < MAX_ITERATION_DEPTH)
                {
                    float iterNormalized = iterationsArray[i] * colorScale;
                    r = (sf::Uint8)(iterNormalized / 2);
                    g = (sf::Uint8)(iterNormalized * 2 + 2);
                    b = (sf::Uint8)(iterNormalized * 2 + 5);
                }

                int pixelIndex = (screenY * WINDOW_WIDTH + screenX + i) * 4;
                pixels[pixelIndex + 0] = r;
                pixels[pixelIndex + 1] = g;
                pixels[pixelIndex + 2] = b;
                pixels[pixelIndex + 3] = 255;
            }

            _cx = _mm256_add_ps(_cx, _8dx);
        }
        _cy = _mm256_add_ps(_cy, _dy);
    }
}

void mandelbrot_vectorized(sf::Uint8* pixels, float magnifier, float shiftX)
{
    mandelbrot_vectorized_internal(pixels, magnifier, shiftX, 0.0f, WINDOW_HEIGHT);
}

void mandelbrot_multithreaded(sf::Uint8* pixels, float magnifier, float shiftX)
{
    std::vector<std::thread> threads;

    for (int j = 0; j < N_THREADS; ++j)
    {
        threads.emplace_back([&, j]()
        {
            int startY = WINDOW_HEIGHT / N_THREADS * j;
            int endY   = WINDOW_HEIGHT / N_THREADS * (j + 1);
            mandelbrot_vectorized_internal(pixels, magnifier, shiftX, startY, endY);
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}

void mandelbrot_unfair(sf::Uint8* pixels, float magnifier, float shiftX)
{
    static_assert(WINDOW_HEIGHT == 1080);
    int coords[] = {0, 295, 365, 421, 466, 503, 540, 577, 613, 658, 714, 784, 1080};

    std::vector<std::thread> threads;

    for (int j = 0; j < N_THREADS; j++)
    {
        threads.emplace_back([&, j]()
        {
            int startY = coords[j];
            int endY   = coords[j + 1];
            mandelbrot_vectorized_internal(pixels, magnifier, shiftX, startY, endY);
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}

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

void mandelbrot_pool(sf::Uint8* pixels, float magnifier, float shiftX)
{
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
