# Modern C++ Smart Pointer Implementation

This project is a custom implementation of modern C++ smart pointers, created from scratch. It aims to replicate the core functionality of `std::unique_ptr`, `std::shared_ptr`, and `std::weak_ptr` using C++17.

## Features

- **`my_ptr::unique_ptr`**: An exclusive ownership smart pointer with support for custom deleters and `make_unique`.
- **`my_ptr::shared_ptr`**: A reference-counted smart pointer that manages shared ownership of an object. Includes `make_shared` for optimized allocation.
- **`my_ptr::weak_ptr`**: A non-owning smart pointer that holds a weak reference to an object managed by a `shared_ptr`.

## Build Instructions

This project uses CMake as its build system.

### Prerequisites

- A C++17 compliant compiler (e.g., MSVC, GCC, Clang)
- CMake (version 3.10 or later)

### Steps to Build

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd smart_pointer
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure with CMake:**
    ```bash
    cmake ..
    ```

4.  **Compile the project:**
    ```bash
    cmake --build . --config Release
    ```

This will generate two executables in the `build/Release` directory:
- `test_smart_ptr.exe`: The test suite for all smart pointer implementations.
- `benchmark.exe`: The performance benchmark comparing `my_ptr` against `std` smart pointers.

## Usage Example

Here is a basic example of how to use the custom smart pointers. Remember to include the main header file.

```cpp
#include "include/memory.hpp"
#include <iostream>

int main() {
    // Using my_ptr::unique_ptr
    my_ptr::unique_ptr<int> u_ptr = my_ptr::make_unique<int>(42);
    std::cout << "unique_ptr value: " << *u_ptr << std::endl;

    // Using my_ptr::shared_ptr
    my_ptr::shared_ptr<std::string> s_ptr = my_ptr::make_shared<std::string>("Hello, World!");
    my_ptr::shared_ptr<std::string> s_ptr2 = s_ptr;
    
    std::cout << "shared_ptr value: " << *s_ptr2 << std::endl;
    std::cout << "shared_ptr use count: " << s_ptr.use_count() << std::endl;

    return 0;
}
```

## Running Tests and Benchmarks

After building the project, you can run the tests and benchmarks from the `build` directory.

### Run Unit Tests

Execute the test suite to verify the correctness of the implementation:

```bash
.\Release\test_smart_ptr.exe
```
All tests should pass, confirming that the smart pointers are functioning as expected in a single-threaded environment.

### Run Performance Benchmark

Execute the benchmark to compare the performance of `my_ptr` smart pointers against the standard library's implementation:

```bash
.\Release\benchmark.exe
```

### Performance Notes

- **`unique_ptr`**: The performance is highly competitive and often slightly faster than `std::unique_ptr` due to its simpler implementation.
- **`shared_ptr`**: The performance is also very close to `std::shared_ptr` in single-threaded scenarios.
- **Thread Safety**: The current implementation of `shared_ptr` is **not fully thread-safe**. While it uses atomic operations for reference counting, it fails under heavy multithreaded contention. Further work is required to implement a fully robust, thread-safe control block.
