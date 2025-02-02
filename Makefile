CXX := clang++
CC := clang++
CXXFLAGS := -Wall -Wextra -O3 -march=native -fopenmp
CFLAGS := -Wall -Wextra -O3 -march=native
INCLUDE_DIRS := -Iinclude -IThread-Pool
LDFLAGS := -lpthread -lsfml-graphics -lsfml-window -lsfml-system -lomp
BUILD_DIR := build

C_SOURCES := Thread-Pool/thread_pool.c
CPP_SOURCES := $(wildcard source/*.cpp)
C_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
CPP_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(CPP_SOURCES:.cpp=.o)))
OBJS := $(C_OBJECTS) $(CPP_OBJECTS)
EXECUTABLE := mandelbrot

all: $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir -p $@

$(EXECUTABLE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: Thread-Pool/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/%.o: source/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE)

.PHONY: all clean
