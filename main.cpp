#include <SFML/Graphics.hpp>
#include <immintrin.h>
#include <thread>

#include <CL/cl.h>
struct MandelbrotGPU
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem cy_buffer, cx_buffer, result_buffer;
    float* cyVec, * cxVec;
    int* result;
};

// cringe
MandelbrotGPU gpu = {};

const int WINDOW_WIDTH  = 1920;
const int WINDOW_HEIGHT = 1080;
const int N_THREADS = 12;

const int   MAX_ITERATION_DEPTH = 256;
const float MAX_RADIUS          = 50.0;

void mandelbrot_naive     (sf::Uint8* pixels, float magnifier, float shiftX);
void mandelbrot_vectorized(sf::Uint8* pixels, float magnifier, float shiftX);
void mandelbrot_multithreaded(sf::Uint8* pixels, float magnifier, float shiftX);

MandelbrotGPU mandelbrot_gpu_setup();
void mandelbrot_gpu(sf::Uint8* pixels, float magnifier, float shiftX);
void mandelbrot_gpu_destr(MandelbrotGPU* gpu);

static void test_implem(sf::Uint8* pixels, void (*func)(sf::Uint8*, float, float),
                            const char* name, int nTests)
{
    printf("Testing %-50s: ", name);
    timespec start = {};
    timespec end = {};
    timespec_get(&start, TIME_UTC);

    for (int i = 0; i < nTests; i++)
        func(pixels, 1.0f, 0.0f);

    timespec_get(&end, TIME_UTC);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("%.5f seconds\n", elapsed);
}

void test(sf::Uint8* pixels)
{
    const int nTests = 150;
    test_implem(pixels, mandelbrot_naive,         "naive"     ,    nTests);
    test_implem(pixels, mandelbrot_vectorized,    "vectorized",    nTests);
    test_implem(pixels, mandelbrot_multithreaded, "multithreaded", nTests);
    test_implem(pixels, mandelbrot_gpu,           "gpu",           nTests);
}

int main()
{
    gpu = mandelbrot_gpu_setup();
    sf::Uint8* pixels = new sf::Uint8[WINDOW_WIDTH * WINDOW_HEIGHT * 4];
    test(pixels);

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Mondelbrot");

    sf::Texture texture;
    texture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    texture.update(pixels);

    sf::Sprite sprite(texture);

    double scale  = 1.0f;
    double shiftX = 0.0f;
    
    MandelbrotGPU gpu = mandelbrot_gpu_setup();

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
        
        mandelbrot_multithreaded(pixels, scale, shiftX);
        texture.update(pixels);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    mandelbrot_gpu_destr(&gpu);
    delete[] pixels;

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

            sf::Uint8 r, g, b;
            if (iteration < MAX_ITERATION_DEPTH)
            {
                float iterNormalized = iteration * colorScale;
                r = (sf::Uint8)(iterNormalized / 2);
                g = (sf::Uint8)(iterNormalized * 2 + 2);
                b = (sf::Uint8)(iterNormalized * 2 + 5);
            }
            else 
            {
                r = 0;
                g = 0;
                b = 0;
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
                else 
                {
                    r = 0;
                    g = 0;
                    b = 0;
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

#include <CL/opencl.hpp>

const char* kernel_source = R"(
    __kernel void solve_mandelbrot(__global float const * real,
                                   __global float const * imag,
                                   __global int * result)
    {
        // Get Parallel Index
        unsigned int i = get_global_id(0);

        float x = real[i]; // Real Component
        float y = imag[i]; // Imaginary Component
        int   n = 0;       // Tracks Color Information

        // Compute the Mandelbrot Set
        while ((x * x + y * y <= 2 * 2) && n < 256)
        {
            float xtemp = x * x - y * y + real[i];
            y = 2 * x * y + imag[i];
            x = xtemp;
            n++;
        }

        // Write Results to Output Arrays
        result[i] = x * x + y * y <= 2 * 2 ? -1 : n;
    }
)";

MandelbrotGPU mandelbrot_gpu_setup() {
    MandelbrotGPU gpu = {};

    // Get the first available OpenCL platform
    gpu.err = clGetPlatformIDs(1, &gpu.platform, NULL);
    if (gpu.err != CL_SUCCESS) {
        printf("Error getting platform ID: %d\n", gpu.err);
        return gpu;
    }

    // Get the first available CPU device
    gpu.err = clGetDeviceIDs(gpu.platform, CL_DEVICE_TYPE_CPU, 1, &gpu.device, NULL);
    if (gpu.err != CL_SUCCESS) {
        printf("Error getting device ID: %d\n", gpu.err);
        return gpu;
    }

    // Create an OpenCL context
    gpu.context = clCreateContext(NULL, 1, &gpu.device, NULL, NULL, &gpu.err);
    if (gpu.err != CL_SUCCESS) {
        printf("Error creating context: %d\n", gpu.err);
        return gpu;
    }

    // Create a command queue
    gpu.queue = clCreateCommandQueue(gpu.context, gpu.device, 0, &gpu.err);
    if (gpu.err != CL_SUCCESS) {
        printf("Error creating command queue: %d\n", gpu.err);
        clReleaseContext(gpu.context);
        return gpu;
    }

    // Create the program from the kernel source
    gpu.program = clCreateProgramWithSource(gpu.context, 1, (const char **)&kernel_source, NULL, &gpu.err);
    if (gpu.err != CL_SUCCESS) {
        printf("Error creating program: %d\n", gpu.err);
        clReleaseCommandQueue(gpu.queue);
        clReleaseContext(gpu.context);
        return gpu;
    }
    
    // Build the program
    gpu.err = clBuildProgram(gpu.program, 0, NULL, NULL, NULL, NULL);
    if (gpu.err != CL_SUCCESS) {
        size_t log_size;
        char *log;
        clGetProgramBuildInfo(gpu.program, gpu.device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        log = (char *)malloc(log_size + 1);
        clGetProgramBuildInfo(gpu.program, gpu.device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        log[log_size] = '\0';
        printf("Build log:\n%s\n", log);
        free(log);
        clReleaseProgram(gpu.program);
        clReleaseCommandQueue(gpu.queue);
        clReleaseContext(gpu.context);
        return gpu;
    }

    // Create the kernel
    gpu.kernel = clCreateKernel(gpu.program, "solve_mandelbrot", &gpu.err);
    if (gpu.err != CL_SUCCESS) {
        printf("Error creating kernel: %d\n", gpu.err);
        clReleaseProgram(gpu.program);
        clReleaseCommandQueue(gpu.queue);
        clReleaseContext(gpu.context);
        return gpu;
    }
    size_t nElems = WINDOW_HEIGHT * WINDOW_WIDTH;
    float* cxVec = (float*) calloc(sizeof(float), nElems);
    float* cyVec = (float*) calloc(sizeof(float), nElems);
    int * result = (int*)   calloc(sizeof(int)  , nElems);
    if (!cxVec || !cyVec || !result)
    {
        printf("Error creating kernel: %d\n", gpu.err);
        clReleaseProgram(gpu.program);
        clReleaseCommandQueue(gpu.queue);
        clReleaseContext(gpu.context);
        return gpu;
    }
    gpu.cxVec = cxVec;
    gpu.cyVec = cyVec;
    gpu.result = result;

    return gpu;
}


void mandelbrot_gpu(sf::Uint8* pixels, float magnifier, float shiftX)
{
    size_t nElems = WINDOW_HEIGHT * WINDOW_WIDTH;

    // Main logic
    const float radius2 = MAX_RADIUS * MAX_RADIUS;
    const float scale = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    shiftX -= 0.5f;
    magnifier -= 0.3f;

    const float invMagnifier = 1.0f / magnifier;
    const float colorScale = 255.0f / MAX_ITERATION_DEPTH;

    const float dx = scale * invMagnifier * (2.0f / WINDOW_WIDTH);
    const float dy = invMagnifier * (2.0f / WINDOW_HEIGHT);

    size_t bufferSize = WINDOW_HEIGHT * WINDOW_WIDTH * sizeof(float);
    size_t resultSize = WINDOW_HEIGHT * WINDOW_WIDTH * sizeof(int);
    size_t elemsSize  = WINDOW_HEIGHT * WINDOW_WIDTH;


    float cy = -1.0f * invMagnifier;
    for (int screenY = 0; screenY < WINDOW_HEIGHT; ++screenY, cy += dy)
    {
        float cx = shiftX - scale * invMagnifier;
        for (int screenX = 0; screenX < WINDOW_WIDTH; ++screenX, cx += dx)
        {
            gpu.cyVec[screenY * WINDOW_WIDTH + screenX] = cx;
            gpu.cxVec[screenY * WINDOW_WIDTH + screenX] = cy;
        }
    }

    
    // Create input and output buffers
    cl_mem cy_buffer     = clCreateBuffer(gpu.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nElems * sizeof(float), gpu.cyVec, &gpu.err);
    cl_mem cx_buffer     = clCreateBuffer(gpu.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nElems * sizeof(float), gpu.cxVec, &gpu.err);
    cl_mem result_buffer = clCreateBuffer(gpu.context, CL_MEM_WRITE_ONLY, nElems * sizeof(int), NULL, &gpu.err);
    if (gpu.err != CL_SUCCESS) {
        printf("Error creating buffers: %d\n", gpu.err);
        clReleaseKernel(gpu.kernel);
        clReleaseProgram(gpu.program);
        clReleaseCommandQueue(gpu.queue);
        clReleaseContext(gpu.context);
        return;
    }

    // Set kernel arguments
    gpu.err  = clSetKernelArg(gpu.kernel, 0, sizeof(cl_mem), &cy_buffer);
    gpu.err |= clSetKernelArg(gpu.kernel, 1, sizeof(cl_mem), &cx_buffer);
    gpu.err |= clSetKernelArg(gpu.kernel, 2, sizeof(cl_mem), &result_buffer);
    if (gpu.err != CL_SUCCESS) {
        printf("Error setting kernel arguments: %d\n", gpu.err);
        clReleaseMemObject(cy_buffer);
        clReleaseMemObject(cx_buffer);
        clReleaseMemObject(result_buffer);
        clReleaseKernel(gpu.kernel);
        clReleaseProgram(gpu.program);
        clReleaseCommandQueue(gpu.queue);
        clReleaseContext(gpu.context);
        return;
    }

    // Execute the kernel
    size_t global_size = nElems;
    gpu.err = clEnqueueNDRangeKernel(gpu.queue, gpu.kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
    if (gpu.err != CL_SUCCESS) {
        printf("Error executing kernel: %d\n", gpu.err);
        clReleaseMemObject(cy_buffer);
        clReleaseMemObject(cx_buffer);
        clReleaseMemObject(result_buffer);
        clReleaseKernel(gpu.kernel);
        clReleaseProgram(gpu.program);
        clReleaseCommandQueue(gpu.queue);
        clReleaseContext(gpu.context);
        return;
    }

    // Read the result buffer
    gpu.err = clEnqueueReadBuffer(gpu.queue, result_buffer, CL_TRUE, 0, nElems * sizeof(int), gpu.result, 0, NULL, NULL);
    if (gpu.err != CL_SUCCESS) {
        printf("Error reading result buffer: %d\n", gpu.err);
        free(gpu.cyVec);
        free(gpu.cxVec);
        free(gpu.result);
        clReleaseMemObject(cy_buffer);
        clReleaseMemObject(cx_buffer);
        clReleaseMemObject(result_buffer);
        clReleaseKernel(gpu.kernel);
        clReleaseProgram(gpu.program);
        clReleaseCommandQueue(gpu.queue);
        clReleaseContext(gpu.context);
        return;
    }

    // OPENCL SHIT ENDS

    for (int pixel = 0; pixel < WINDOW_WIDTH * WINDOW_HEIGHT; pixel++)
    {
        sf::Uint8 r = 0;
        sf::Uint8 g = 0;
        sf::Uint8 b = 0;

        if (gpu.result[pixel] != -1)
        {
            float iterNormalized = gpu.result[pixel] * colorScale;
            r = (sf::Uint8)(iterNormalized / 2);
            g = (sf::Uint8)(iterNormalized * 2 + 2);
            b = (sf::Uint8)(iterNormalized * 2 + 5);
        }

        pixels[4 * pixel + 0] = r;
        pixels[4 * pixel + 1] = g;
        pixels[4 * pixel + 2] = b;
        pixels[4 * pixel + 3] = 255;
    }
}


void mandelbrot_gpu_destr(MandelbrotGPU* gpu)
{
    free(gpu->cxVec);
    free(gpu->cyVec);
    free(gpu->result);

    clReleaseMemObject(gpu->cy_buffer);
    clReleaseMemObject(gpu->cx_buffer);
    clReleaseMemObject(gpu->result_buffer);
    clReleaseKernel(gpu->kernel);
    clReleaseProgram(gpu->program);
    clReleaseCommandQueue(gpu->queue);
    clReleaseContext(gpu->context);
}
