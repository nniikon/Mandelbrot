![](./media/mandelbrot.png)

# Mandelbrot

## Requirements

- **CPU mode**: AVX2 instruction set, clang compiler
- **GPU mode**: CUDA Toolkit (nvcc)
- **Libraries**: SFML 2.6, OpenMP, pthread

## Clone and build

```bash
git clone --recursive https://github.com/nniikon/Mandelbrot.git  
cd Mandelbrot  
make       # CPU build  
make GPU=1 # GPU build  
```

## Run

```bash
./mandelbrot {mode} [number_of_test_iterations]
```

**Modes:**

- **naive** – basic implementation
- **vectorized** – AVX2 optimizations
- **arrayed** – compiler-assisted vectorization
- **openmp** – OpenMP parallelization
- **thread-pool** – custom thread pool
- **cuda** – GPU computation (requires CUDA)
