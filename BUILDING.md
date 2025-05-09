# Building the Trading Engine

## Prerequisites

1. **C++ Compiler**:
   - **Windows**: MSVC (Visual Studio 2019 or later)
   - **Linux**: GCC 10+
   - **macOS**: Clang 10+

2. **CMake** (version 3.16 or higher):
   - **Windows**: Download from [https://cmake.org/download/](https://cmake.org/download/)
   - **Linux**: `sudo apt install cmake` (Ubuntu/Debian) or `sudo yum install cmake` (CentOS/RHEL)
   - **macOS**: `brew install cmake`

## Windows Build Instructions

### Using Visual Studio:

1. Install Visual Studio 2019 or later with "Desktop development with C++" workload
2. Install CMake from the URL above
3. Open a command prompt or PowerShell window
4. Navigate to the project root directory
5. Run:
   ```
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```
6. Open the generated `.sln` file in Visual Studio
7. Build the solution using Visual Studio interface

### Using command line:

1. Open a Developer Command Prompt for Visual Studio
2. Navigate to the project root directory
3. Run:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

## Linux/macOS Build Instructions

1. Open a terminal
2. Navigate to the project root directory
3. Run:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

## Running Tests

After building:

```
cd build
ctest
```

## Common Issues

1. **CMake not found**: Ensure CMake is installed and in your PATH
2. **Compiler not found**: Ensure you have the required compiler installed
3. **C++20 not supported**: Update your compiler to a version that supports C++20

## Building Performance Optimizations

For optimal performance:

1. Build in Release mode:
   ```
   cmake --build . --config Release
   ```

2. Enable compiler-specific optimizations:
   ```
   # GCC/Clang
   cmake .. -DCMAKE_CXX_FLAGS="-march=native -O3"
   
   # MSVC
   cmake .. -DCMAKE_CXX_FLAGS="/O2 /Ot"
   ``` 