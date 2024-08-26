/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

/*************************************************************************/ /**
 * @file
 * @brief  contains the task application class
 * @ingroup  Task Abstraction
 *****************************************************************************/

#pragma once

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include <condition_variable>
#include <functional>
#include <stop_token>
#include <thread>

using namespace std::chrono_literals;

//----------------------------------------------------------------------------
// Declarations
//----------------------------------------------------------------------------
namespace app::task {

struct TaskEvent {
  std::mutex event_mutex;
  std::condition_variable event_condition;
};

using duration_unit = std::chrono::milliseconds;

template <typename T>
using FunctionType = std::function<T(T)>;

/**
 * Event driving task with the earliest possible processing time
 * @tparam ContextType
 * @tparam ConfigType
 */
template <class DurationUnit = std::chrono::milliseconds>
class TaskController {
 public:
  ~TaskController() = default;

  void start(DurationUnit defDuration, DurationUnit zeroDuration, FunctionType<DurationUnit> serveFunction,
             std::stop_token token, TaskEvent& event) {
    m_taskThread =
        std::move(std::thread(TaskController::runTask, defDuration, zeroDuration, serveFunction, token, event));
  };

  void stop() {
    if (m_taskThread.joinable()) {
      m_taskThread.join();
    }
  };

 private:
  static void runTask(DurationUnit defDuration, DurationUnit zeroDuration, FunctionType<DurationUnit> serveFunction,
                      std::stop_token token, TaskEvent& event) {
    auto sooner{defDuration};
    registerStopCallback(token, event);

    while (true) {
      sooner = serveFunction(sooner);
      if (sooner.count() > 0) {
        std::unique_lock lck(event.event_mutex);
        event.event_condition.wait_for(lck, sooner);
      } else {
        sooner = zeroDuration;
      }

      if (token.stop_requested()) {
        break;
      }
    }
  };

  static void registerStopCallback(std::stop_token token, TaskEvent& event) {
    std::stop_callback{token, [&]() {
                         event.event_condition.notify_all();
                       }};
  }

  std::thread m_taskThread;
};
}  // namespace app::task