# Trading Engine

A high-performance, low-latency trading engine implemented in C++20.

## Features

- **Core utilities**: High-resolution timing, lock-free logging, benchmarking tools
- **Order Book**: High-performance limit order book with efficient price level storage
- **Market Simulation**: Simulated market data with configurable latency and slippage
- **Strategy Execution**: Extensible interface for trading strategies (with Python bindings)
- **Networking**: Optional TCP/UDP interfaces (planned)

## Project Structure

```
trading_engine/
├── include/             # Header files
│   ├── core/            # Core utilities
│   ├── orderbook/       # Order book implementation
│   ├── market/          # Market simulation
│   ├── strategy/        # Strategy execution
│   └── network/         # Network interfaces
├── src/                 # Implementation files
│   ├── core/            # Core utilities
│   ├── orderbook/       # Order book implementation
│   ├── market/          # Market simulation
│   ├── strategy/        # Strategy execution
│   └── network/         # Network interfaces
└── test/                # Test files
    ├── core/            # Core tests
    ├── orderbook/       # Order book tests
    ├── market/          # Market simulation tests
    ├── strategy/        # Strategy execution tests
    └── network/         # Network interface tests
```

## Building

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.16+

### Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Running Tests

```bash
cd build
ctest
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.
