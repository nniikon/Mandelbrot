GPU=0

ifeq ($(GPU),0)
CXX := clang++ 
else
CXX := nvcc
endif

FLAGS := -O3
CXXFLAGS := -Wall -Wextra -march=native -fopenmp
NVCCFLAGS := -arch=sm_75 --use_fast_math --compiler-options -march=native --compiler-options -fopenmp -DGPU

INCLUDE_DIRS := -Iinclude -IThread-Pool
LDFLAGS := -lpthread -lsfml-graphics -lsfml-window -lsfml-system -lomp
BUILD_DIR := build

ifeq ($(GPU),1)
LDFLAGS += -lcuda
FLAGS += $(NVCCFLAGS)
else
FLAGS += $(CXXFLAGS)
endif

C_SOURCES := Thread-Pool/thread_pool.c
CPP_SOURCES := $(wildcard source/*.cpp)
CU_SOURCES := source/mandelbrot_cuda.cu
C_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
CPP_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(CPP_SOURCES:.cpp=.o)))
CU_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(CU_SOURCES:.cu=.o)))
OBJS := $(C_OBJECTS) $(CPP_OBJECTS)

ifeq ($(GPU),1)
OBJS += $(CU_OBJECTS)
endif
EXECUTABLE := mandelbrot

all: $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir -p $@

$(EXECUTABLE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: Thread-Pool/%.c | $(BUILD_DIR)
	gcc $(FLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/%.o: source/%.cpp | $(BUILD_DIR)
	$(CXX) $(FLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/%.o: source/%.cu | $(BUILD_DIR)
	$(CXX) $(FLAGS) $(INCLUDE_DIRS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE)

.PHONY: all clean
