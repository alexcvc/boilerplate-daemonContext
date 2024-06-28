#ifndef APP_SV_SUB_INCLUDE_EVENT_HPP
#define APP_SV_SUB_INCLUDE_EVENT_HPP

#include <condition_variable>
#include <mutex>

/**
 * @brief Event plan structure
 */
struct Event {
 public:
  void lock() { event_mutex.lock(); }

  void unlock() { event_mutex.unlock(); }

  std::mutex event_mutex;
  std::condition_variable event_condition;
};

#endif /* APP_SV_SUB_INCLUDE_EVENT_HPP */
