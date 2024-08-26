#include <functional>
#include <iostream>
#include <stop_token>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

class ThreadController {
 public:
  using ThreadFunction = std::function<void(size_t index, std::stop_token)>;

  ThreadController() = default;
  ~ThreadController() {
    stopAll();
  }

  void startThreads(ThreadFunction function, size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
      createThread(function, i);
    }
  }

  void stopAll() {
    for (auto& stopSource : stopSources_) {
      if (stopSource.stop_possible()) {
        stopSource.request_stop();
      }
    }
    joinAndClearThreads();
  }

 private:
  void createThread(ThreadFunction function, size_t index) {
    std::stop_source stopSource;
    std::jthread t(function, index, stopSource.get_token());
    threads_.emplace_back(std::move(t));
    stopSources_.emplace_back(std::move(stopSource));
  }

  void joinAndClearThreads() {
    for (auto& t : threads_) {
      if (t.joinable()) {
        t.join();
      }
    }
    threads_.clear();
    stopSources_.clear();
  }

  std::vector<std::jthread> threads_;
  std::vector<std::stop_source> stopSources_;
};

static const size_t NUM_THREADS = 1000;

void threadTask(size_t index, std::stop_token stopToken) {
  while (!stopToken.stop_requested()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    spdlog::info("Working in thread <{}>...", index);
  }
  spdlog::warn("Thread <{}> is stopping.", index);
}

int main() {
  ThreadController controller;

  controller.startThreads(threadTask, NUM_THREADS);

  std::this_thread::sleep_for(std::chrono::seconds(3));

  controller.stopAll();

  std::cout << "All threads stopped." << std::endl;
  return 0;
}