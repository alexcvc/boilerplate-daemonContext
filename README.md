# Another C++ daemon boilerplate

## Abstract

This contains another C++ daemon boilerplate implementation

This contains another C++ daemon boilerplate implementation.
A focus of this implementation is the embedded Linux applications.

There are several optional features, such as foreground execution and a test console.
The C++ compiler must support at least C++11, a C++20 compiler is preferred for future enhancements.

In the TODO plan there are the following:

1) Examples of using and extending this boilerplate for a real application with a separate implementation of an independent application
   context;
2) secure use in multithreaded applications;
3) application restarts without a stop and starts daemon.
4) examples an abstract context application for protocol converter f.e. IEC61850, IE60870-5-10X;
5) examples of how to use these codes in IoT applications with Raspberry and BeagleBoard.

## Important Notes for C++11/14/17

### `std::stop_callback`:

- Introduced in C++20 as part of the cooperative cancellation mechanism with `std::stop_token`.
- It allows registering a callback function that automatically executes when a `std::stop_source` requests cancellation.
- It is useful for cleanup or performing graceful shutdown tasks when cancellation occurs.

### `get_token()` in C++11/14/17:

- In C++20, `std::stop_source` provides the `get_token()` method to obtain a `std::stop_token`.
- In C++11/14/17, there is no native support for `std::stop_token` or `get_token()`.
- To emulate similar functionality, use shared control objects such as `std::shared_ptr<std::atomic<bool>>`.
- These can be passed to and checked within threads or tasks to determine if a cancellation request has been made.

### Summary:

- `std::stop_callback` and `get_token()` are features of C++20.
- In C++11/14/17, manage cancellation manually using shared flags and periodically check them within your tasks.

Example usage in C++20:

```cpp
  #include <stop_token>
  #include <iostream>
  #include <thread>

  void cleanup() {
      std::cout << "Cleanup callback invoked." << std::endl;
  }

  int main() {
      std::stop_source source;
      std::stop_token token = source.get_token();

      // Register a callback
      std::stop_callback callback(token, cleanup);

      // Request cancellation
      source.request_stop();

      // The callback will be invoked automatically
  }
  ```

Example in C++11/14/17 using a shared atomic flag:

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

// Emulate a stop token with an atomic boolean
struct StopToken {
    std::shared_ptr<std::atomic<bool>> stop_flag;

    explicit StopToken() : stop_flag(std::make_shared<std::atomic<bool>>(false)) {}

    bool stop_requested() const { return stop_flag->load(); }

    void request_stop() { stop_flag->store(true); }
};

// Usage in a thread
void task(StopToken& token) {
    while (!token.stop_requested()) {
        // Perform work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Working..." << std::endl;
    }
    std::cout << "Stopped." << std::endl;
}

int main() {
    StopToken token;

    std::thread worker(task, std::ref(token));

    // Simulate some work in main thread
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Request stop
    token.request_stop();

    worker.join();
}
```
