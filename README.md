# Another C++ daemon boilerplate

## Abstract

This contains another C++ daemon boilerplate implementation

This contains another C++ daemon boilerplate implementation.
A focus of this implementation the embedded Linux applications.

There are several optional features such as foreground execution and a test console.
The C++ compiler must support at least C++11, a C++20 compiler is preferred for future enhancements.

In the TODO plan there are followings:

1) Examples of using and extending this boilerplate for a real application with a separate implementation of an independent application
   context;
2) secure use in multithreaded applications;
3) application restart without stop and start daemon.
4) examples an abstract context application for protocol converter f.e. IEC61850, IE60870-5-10X;
5) examples of how to use these code in IoT applications with Raspberry and BeagleBoard.

## Important notes for C++17

## `std::stop_callback` with `get_token()` under C++17

### `std::stop_callback`:

- Introduced in C++20 as part of the cooperative cancellation mechanism with `std::stop_token`.
- It registers a callback function that automatically executes when a `std::stop_source` requests cancellation.
- It helps with clean-up or graceful shutdown when cancellation occurs.

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

**Note:** No `std::stop_callback` in C++17.

### `get_token()`:

- In C++20, `std::stop_source` provides `get_token()`.
- In C++17, no such mechanism exists.
- The common approach in C++17 is to emulate cancellation tokens with shared control objects like `std::atomic<bool>`.

To emulate, you can use a shared `std::shared_ptr<std::atomic<bool>>` acting as a manual cancellation token.
Inside your thread or task, check the flag periodically to determine if cancellation was requested.

Example:

```c++
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

### Summary:

- No `get_token()` in C++17.
- Use shared `std::shared_ptr<std::atomic<bool>>`.
- Pass it to threads/tasks and check whether cancellation has been requested.

