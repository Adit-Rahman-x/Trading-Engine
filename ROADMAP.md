# Trading Engine Roadmap

This document outlines the development roadmap for the Trading Engine project.

## Phase 1: Core Engine Foundations ✅

- [x] Project structure with CMake setup
- [x] High-resolution timing utilities
  - Nanosecond precision timing
  - Wall clock and elapsed time measurements
  - Timestamp formatting for logging
- [x] Lock-free logging system
  - Multiple log levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
  - Async logging with dedicated thread
  - Lock-free ring buffer for minimal overhead
  - File and console output
- [x] Benchmarking utilities
  - Iteration-based benchmarking
  - Duration-based benchmarking
  - Statistical measurements (min, max, mean, stddev, percentiles)
  - Convenience macros for code timing
- [x] Unit tests for core modules

## Phase 2: Order Book Engine 🔜

- [ ] Basic data types
  - Order ID
  - Price and quantity (fixed point for determinism)
  - Security identifiers
- [ ] Order types
  - Limit orders
  - Market orders
  - Cancel orders
  - Modify orders
- [ ] Price level abstraction
  - Efficient storage (red-black tree or sorted flat map)
  - Price level queues with FIFO semantics
- [ ] Order book implementation
  - Bid/ask sides
  - Fast lookup by price and order ID
  - Best bid/ask tracking
- [ ] Matching engine
  - Order submission
  - Order cancellation
  - Order modifications
  - Full and partial fills
  - Match reporting
- [ ] Benchmarking
  - Throughput (orders/second)
  - Latency (order-to-match time)
  - Memory usage

## Phase 3: Market Simulation Layer

- [ ] Market data structures
  - Order book updates
  - Trade prints
  - Market state snapshots
- [ ] Event generation
  - Random order generation
  - Data replay from file (CSV, FIX)
  - Configurable rates and patterns
- [ ] Latency simulation
  - Per-order latency models
  - Configurable jitter
- [ ] Market impact simulation
  - Slippage models
  - Price impact
- [ ] Event bus
  - Pub/sub architecture
  - Order/trade/book update events

## Phase 4: Strategy Execution Interface

- [ ] Strategy base interface
  - on_book_update hook
  - on_order_fill hook
  - on_tick hook (time-based)
- [ ] Strategy management
  - Registration/unregistration
  - Parameter configuration
  - State tracking
- [ ] Position tracking
  - Real-time P&L
  - Risk limits
  - Position modeling
- [ ] Python bindings (optional)
  - pybind11 integration
  - Python strategy examples
  - Minimal data copying

## Phase 5: Networking (Optional)

- [ ] Protocol design
  - Order message format
  - Market data format
  - Status/acknowledgments
- [ ] TCP/UDP server
  - Client connections
  - Authentication
  - Message parsing
- [ ] WebSocket interface
  - Real-time data streaming
  - Order status updates
- [ ] FIX protocol adapter (optional)
  - Basic FIX message support
  - FIX session management

## Ongoing Tasks

- Performance optimization
- Continuous testing
- Documentation
- Example strategies 