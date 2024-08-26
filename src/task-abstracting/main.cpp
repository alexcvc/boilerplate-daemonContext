/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "taskController.hpp"

using namespace std::chrono_literals;

//----------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Typedefs, enums, unions, variables
//----------------------------------------------------------------------------
enum class handleConsoleType {
  none,
  reload,
  exit,
};

struct AppContext {
  std::chrono::milliseconds process(std::chrono::milliseconds ms) {
    if (ms.count() > 4000) {
      ms = 1000ms;
    } else {
      ms += 1000ms;
    }
    spdlog::info("Process for {} ms", ms.count());
    return ms;
  }
};

/*************************************************************************/ /**
 * @brief handle console input
 *****************************************************************************/
bool handle_console() {
  auto key = getchar();
  switch (key) {
    case 'q':
      return true;
    case '?':
    case 'h':
      fprintf(stderr, "Application test console:\n");
      fprintf(stderr, " q   -  quit from application.\n");
      fprintf(stderr, " h|? -  this information.\n");
      break;
    default:;
  }
  return false;
}

/*************************************************************************/ /**
 * @file main.c
 * @brief This is the main entry point for the application.
 *****************************************************************************/
int main(int argc, char** argv) {
  // The Daemon class is a singleton to avoid be instantiated more than once
  std::stop_source stop_src;                                  ///< stop token for the main loop
  app::task::TaskEvent task_event;                            ///< The task event
  app::task::TaskController<std::chrono::milliseconds> task;  ///< The task context

  task.start(
      1000ms, 1ms,
      [](std::chrono::milliseconds soon) {
        if (soon.count() >= 4000) {
          soon = 0ms;
        } else {
          soon += 1000ms;
        }
        spdlog::info("Process for {} ms", soon.count());
        return soon;
      },
      stop_src.get_token(), std::ref(task_event));

  // Main loop
  std::cout << "Press the h key to display the Console Menu..." << std::endl;

  // Daemon main loop
  while (true) {
    if (handle_console()) {
      break;
    }
  }

  // set token to stop all worker
  stop_src.request_stop();

  spdlog::info("The daemon process is stopping");

  // wakeup all tasks
  task.stop();

  spdlog::info("The daemon process ended successfully");

  return EXIT_SUCCESS;
}