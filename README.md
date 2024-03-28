![](./media/mandelbrot.png)

# Mandelbrot Set

The Mandelbrot Set can be defined using this simple pseudocode algorithm.

```
for each pixel (x0, y0) on the screen:
    x := 0.0
    y := 0.0
    iteration := 0
    max_iteration := 256
    while (x^2 + y^2 ≤ Radius^2 AND iteration < Max_iteration):
        xtemp := x^2 - y^2 + x0
        y := 2*x*y + y0
        x := xtemp
        iteration++
```

## System

I'm using an AMD Ryzen 5 5600H CPU with an integrated GPU for the tests.

Debian Clang 16.0.6 with the -O3 flag is used as a compiler.

## Naive Approach

Let's rewrite the pseudocode in C.

I have also applied various optimization techniques to make the code run faster. Those are basic and can be found anywhere on the Internet. We are not going to discuss those.

The results show 1170 * 10^9 ticks per 2500 frames, which is very approximately around 7 FPS.

Note that the ticks are more precise and FPS is only given for a better understanding of fast the approach is.

## Vectorized Approach

The most significant feature of this algorithm is that it can be parallelized as much as you would like.

For example, if you have 1920*1080 CPUs, they can simultaneously calculate each frame and output the result instantaneously.

Modern CPUs have so-called SIMD instructions (Single Instruction Multiple Data). Those can be very handy since they can perform multiple operations at once.

My CPU supports AVX2 instructions, meaning that we can perform up to 8 floating-point multiplications at once.

Let's rewrite our code and test it again.

The results are 180*10^9, or around 45 FPS.

However, we were expecting an 8 times increase and 49 FPS. Where did we lose the 4 FPS?

First, the time it takes to calculate a vector of 8 floats is defined by the slowest pixel, which can ruin the performance.

Second, we still have to look up colors for each pixel one by one.

But in reality, we came close to the theoretical limit, which is quite good.

## Multithreading Approach

As far as I know, we have hit the limit of single-thread speed, so let's add more threads.

My CPU has 6 physical cores and supports 12 threads. We will use them all. We are going to split our screen horizontally into 12 pieces and calculate them in parallel.

![](./media/mandelbrot1.png)

The results show 43 * 10^9 ticks or around 190 FPS.

Again, our expectations were incorrect, and we only gained 4 times extra frames instead of 12.

Similar to the previous case, the total speed is defined by the slowest thread. And if the 8 neighboring pixels had approximately the same time to compute, the threads were spread even more poorly.

The threads that are calculating the central parts are working non-stop, and the first ones are generally resting.

| Thread number | Idle   time |
|:-------------:|:-----------:|
| 1             | 92%         |
| 2             | 90%         |
| 3             | 84%         |
| 4             | 63%         |
| 5             | 44%         |
| 6             | 10%         |

That leads us to the next approach.

## Thread Pool Approach

Let's separate the screen into small chunks and put them in the queue. Then, each thread can take one job from the queue and complete it.
Thus, by distributing the computations more evenly, we reduce the idle time of each thread.

This standard approach of handling embarrassingly parallel problems is called a thread pool.

In order to implement it, I created a simple [thread pool library](https://github.com/nniikon/Thread-Pool). 

Let's test it. The results show 25 * 10^9 ticks or around 330 FPS.

This is around 7 times faster than the vectorized approach, which is a great improvement, especially considering that my CPU only has 6 cores.

## Conclusion
After exploring several optimization approaches for the Mandelbrot set calculation, the following results were achieved:
| Approach              | Naive  | Vectorized | Multithreading | Multithreading (thread pool) |
|-----------------------|:------:|:----------:|:--------------:|:----------------------------:|
| FPS (approximately)   | 7 (1x) | 45 (6.5x)  | 179 (25.6x)    | 330 (46.8x)                  |
| Ticks * 10^9          | 1170   | 180        | 43             | 25                           |

Thanks for reading!
