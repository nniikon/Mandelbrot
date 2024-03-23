# Mandelbrot set

Mandelbrot set can be defined using this simple pseudocode algorithm.
```
for each pixel (Px, Py) on the screen do
    x0 := scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 0.47))
    y0 := scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1.12, 1.12))
    x := 0.0
    y := 0.0
    iteration := 0
    max_iteration := 1000
    while (x^2 + y^2 ≤ 2^2 AND iteration < max_iteration) do
        xtemp := x^2 - y^2 + x0
        y := 2*x*y + y0
        x := xtemp
        iteration := iteration + 1
```
## System
I'm using R5 5600H with an integrated GPU for the tests.
Clangd with -O3 flag is used as a compiler.

## Naive Approach
Let's just rewrite the pseudocode in C. 

I also applied various optimization techniks to make the code run faster, but those are basic and could be found anywhere on the Internet and we're not going to discuss those.

The resulting time is 144.0 seconds per 1000 frames that is around 7 FPS. 

## Vectorized Approach
The most significant feature of this algorithm is that it can be paralleled as much as you'd like.

For example, if you have 1920*1080 CPUs, they can simultaneously calculate each frame and output the result instantaneously.

Modern CPUs have so called SIMD instructions (Single Instruction Multiple Data) and those can be very handy, since they can perform multiple operations at once.

My CPU supports AVX2 instructions, that means that we can perform up to 8 floating-point multiplications at once. 

Let's rewrite our code and test it again.

The resulting time is 24.0 seconds or around 42 FPS.

But we were expecting 8 times increase and 49 FPS. Where did we lose the 7 FPS?

First of all, the time it takes to calculate a vector of 8 floats is defined by the slowest pixel, which can ruin the perfomance.

Second of all, we still have to look up colors for each pixel one by one. 

But actually we came close to the theoretical limit, which is pretty good.

## Multithreading Approach
For as far as I know, we've hit the limit of a single-thread speed, so let's just add more threads.

My CPU has 12 threads. So let's use them all. We're going to split our screen horizontally into 12 peaces and calculate them in parallel.

The resulting time is 5.6 seconds or around 179 FPS.

And again our expectations were incorrect and we only gained 4 times extra frames instead of 12.

Similarly to the previous case, the total speed is defined by the slowest thread. And if the 8 neigbouring pixels had approximatly the same time to compute, the threads were spread even more worse.

The threads that are calculating the central parts are working non-stop and the first once are generally resting.

| Thread number | Idling time |
|:-------------:|:-----------:|
| 1             | 92%         |
| 2             | 90%         |
| 3             | 84%         |
| 4             | 63%         |
| 5             | 44%         |
| 6             | 10%         |

That leads us to the next approach

## Unfair Approach
Since we know the rendering algorithm in advance, we can spread the computations evenly between threads. 

| Thread number |  Surface area |
|:-------------:|:-----------:|
| 1             | 27.3%         |
| 2             | 6.5%         |
| 3             | 5.2%         |
| 4             | 4.2%         |
| 5             | 3.4%         |
| 6             | 10%         |

Just like that we were able to reduce the execution time to 3.52 seconds or 284 FPS, which is a 7 times increase.

And even after evenly spreading the computations between the threads we are still far away from out desired 12x increase in speed.

Since each core on my CPU has two threads, it can't fully work twice as fast:
-- Each core only has a single instance of L3 cache
-- It takes time to create and manage threads
-- Different cores access different parts of memory, 

## Conclusion

| Approach              | Naive  | Vectorized | Multithreading | Multithreading (unfair) |
|-----------------------|:------:|:----------:|:--------------:|:-----------------------:|
| FPS                   | 7      | 42         | 179            | 284                     |
| Time (sec)            | 144.0  | 24.00      | 5.60           | 3.52                    |
