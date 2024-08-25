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

#include <fcntl.h>
#include <getopt.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "appContext.hpp"
#include "daemon.hpp"
#include "daemonConfig.hpp"
#include "version.hpp"

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

struct DaemonEvent {
  std::mutex event_mutex;
  std::condition_variable event_condition;
};

static DaemonEvent daemon_event;

//----------------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Declarations
//----------------------------------------------------------------------------

/***************************************************************************/ /**
 * @brief Displays the version information of the program.
 * @param prog The name of the program.
 * @return None
 ******************************************************************************/
static void show_version(const char* prog) {
  auto verString = version::daemon_with_context::getVersion(true);
  // info
  std::cout << prog << " v." << verString << std::endl;
}

/*************************************************************************/ /**
 * Displays the help message for the program.
 * @param prog The name of the program.
 * @param var The character representing the option with an error. Defaults to 0.
 *****************************************************************************/
static void display_help(const char* prog, std::string_view errorOption = "") {
  if (!errorOption.empty()) {
    std::cerr << "Error in option: " << errorOption << "\n";
  }
  std::cout << "\nUsage: " << prog << " [OPTIONS]\n"
            << std::endl
            << "  -D, --background         start as daemon" << std::endl
            << "  -F, --foreground         start in foreground with test console" << std::endl
            << "  -S, --cfgpath            path to folder with configuration files" << std::endl
            << "  -x, --cfgfile            specified configuration file" << std::endl
            << "  -P, --pidfile            create pid file" << std::endl
            << "  -L, --logfile            specified log file" << std::endl
            << "  -v, --version            version" << std::endl
            << "  -h, --help               this message" << std::endl;
  std::cout << "\nSample command lines:\n" << std::endl;
  std::cout << prog << " -F" << std::endl;
  std::cout << prog << " -D -P /var/run/some.pid" << std::endl;
  std::cout << prog << " -F -S /app/config" << std::endl;
  std::cout << prog << " -D -x /app/config/settings.xml -P /var/run/some.pid" << std::endl;

  if (!errorOption.empty()) {
    exit(EXIT_FAILURE);
  }
}

/*************************************************************************/ /**
 * @brief Processes the command line options passed to the program.
 *
 * This function parses the command line options and sets the corresponding
 * variables based on the provided values. It uses getopt_long function to
 * handle both short and long options.
 *
 * @param argc The number of command line arguments.
 * @param argv The array of command line argument strings.
 * @param config
 *****************************************************************************/
static void process_command_line(int argc, char* argv[], app::DaemonConfig& config) {
  for (;;) {
    int option_index = 0;
    static const char* short_options = "h?vDFP:S:x:L:";
    static const struct option long_options[] = {
        {"help", no_argument, nullptr, 0},
        {"version", no_argument, nullptr, 'v'},
        {"background", no_argument, nullptr, 'D'},
        {"foreground", no_argument, nullptr, 'F'},
        {"pidfile", required_argument, nullptr, 'P'},
        {"cfgpath", required_argument, nullptr, 'S'},
        {"cfgfile", required_argument, nullptr, 'x'},
        {"logfile", required_argument, nullptr, 'L'},
        {nullptr, 0, nullptr, 0},
    };

    int var = getopt_long(argc, argv, short_options, long_options, &option_index);

    if (var == EOF) {
      break;
    }
    switch (var) {
      case 0:
        display_help(argv[0]);
        break;

      case '?':
      case 'h': {
        display_help(argv[0]);
        exit(EXIT_SUCCESS);
      }

      case 'v': {
        show_version(argv[0]);
        exit(EXIT_SUCCESS);
      }

      case 'D':
        config.isDaemon = true;
        config.hasTestConsole = false;
        break;

      case 'F':
        config.isDaemon = false;
        config.hasTestConsole = true;
        break;

      case 'P':
        if (strlen(optarg)) {
          config.pidFile.assign(optarg);
        } else {
          display_help(argv[0], std::to_string(var));
        }
        break;

      case 'S':
        if (strlen(optarg)) {
          config.pathConfigFolder.assign(optarg);
        } else {
          display_help(argv[0], std::to_string(var));
        }
        break;

      case 'x':
        if (strlen(optarg)) {
          config.pathConfigFile.assign(optarg);
        } else {
          display_help(argv[0], std::to_string(var));
        }
        break;

      case 'L':
        if (strlen(optarg)) {
          config.logFile.assign(optarg);
        } else {
          display_help(argv[0], std::to_string(var));
        }
        break;

      default: {
        display_help(argv[0]);
        exit(EXIT_FAILURE);
      }
    }
  }
}

/*************************************************************************/ /**
 * @brief handle console input
 *****************************************************************************/
handleConsoleType handle_console() {
  auto key = getchar();
  switch (key) {
    case 'q':
      return handleConsoleType::exit;
    case 'R':
      return handleConsoleType::reload;
    case 'v':
      // info
      std::cout << " v." << version::daemon_with_context::getVersion(true) << std::endl;
      break;
    case '?':
    case 'h':
      fprintf(stderr, "Application test console:\n");
      fprintf(stderr, " R   -  execute reload functions\n");
      fprintf(stderr, " q   -  quit from application.\n");
      fprintf(stderr, " v   -  version\n");
      fprintf(stderr, " h|? -  this information.\n");
      break;
    default:;
  }
  return handleConsoleType::none;
}

/*************************************************************************/ /**
 * @brief Subscriber main function
 * @desc Threads cannot always actively monitor a stop token.
 * @param lptr - log appender
 * @param cfg_converter - configuration
 * @param token - stop task token
 *****************************************************************************/
void TaskAppContextFunc(app::AppContext& app_context, app::DaemonConfig& daemon_config, std::stop_token token) {
  auto sooner = 1000ms;

  // Register a stop callback
  std::stop_callback stop_cb(token, [&]() {
    // Wake thread on stop request
    daemon_event.event_condition.notify_all();
  });

  spdlog::info("application task started");

  while (true) {
    spdlog::info("application task ticks {} ms", sooner.count());
    // observe serves states
    sooner = app_context.process_executing(sooner);
    if (sooner.count() > 0) {
      // Start of locked block
      std::unique_lock lck(daemon_event.event_mutex);
      daemon_event.event_condition.wait_for(lck, std::chrono::milliseconds(sooner));
    }

    //Stop if requested to stop
    if (token.stop_requested()) {
      spdlog::info("stop requested for an application task");
      break;
    }
  }  // End of while loop

  spdlog::info("application task completed");
}

/*************************************************************************/ /**
 * @brief Check and exit on error
 *****************************************************************************/
void check_and_exit_on_error(std::optional<bool> result, const std::string& error_message) {
  if (result.has_value() && !result.value()) {
     spdlog::warn(error_message + ". Exit");
     exit(EXIT_FAILURE);
   }
}

/*************************************************************************/ /**
 * @file main.c
 * @brief This is the main entry point for the application.
 *****************************************************************************/
int main(int argc, char** argv) {
  // The Daemon class is a singleton to avoid be instantiated more than once
  app::Daemon& daemon = app::Daemon::instance();
  app::DaemonConfig appConfig;  ///< The configuration of the daemon
  app::AppContext appContext;   ///< The application context
  std::stop_source stop_src;    ///< stop token for the main loop
  std::thread taskAppContext;

  //----------------------------------------------------------
  // parse parameters
  //----------------------------------------------------------
  process_command_line(argc, argv, appConfig);

  //----------------------------------------------------------
  // set in daemon all handlers
  //----------------------------------------------------------
  daemon.set_start_function([&]() {
    spdlog::info("Start all function called.");
    return appContext.process_start();
  });

  daemon.set_close_function([&]() {
    spdlog::info("Close all function called.");
    return appContext.process_shutdown();
  });

  daemon.set_reload_function([&]() {
    spdlog::info("Reload function called.");
    return appContext.process_reconfigure();
  });

  daemon.set_user1_function([&]() {
    spdlog::info("User1 function called.");
    return appContext.process_user1();
  });

  daemon.set_user2_function([&]() {
    spdlog::info("User2 function called.");
    return appContext.process_user2();
  });

  //----------------------------------------------------------
  // Check integrity this configuration
  //----------------------------------------------------------
  check_and_exit_on_error(appContext.validate_configuration(appConfig), "configuration mismatch");

  //----------------------------------------------------------
  // Prepare application to start
  //----------------------------------------------------------
  if (auto res = appContext.process_start(); res.has_value() && !res.value()) {
    spdlog::warn("prepare the application for task start failed. Exit");
    exit(EXIT_FAILURE);
  }

  //----------------------------------------------------------
  // Start all
  //----------------------------------------------------------
  if (auto result = daemon.start_all(); result.has_value() && !result.value()) {
    spdlog::warn("Error starting the daemon.");
    return EXIT_FAILURE;
  }

  if (appConfig.isDaemon) {
    if (auto result = daemon.make_daemon(appConfig.pidFile); result.has_value() && !result.value()) {
      spdlog::warn("Error starting the daemon.");
      return EXIT_FAILURE;
    }
  }

  //----------------------------------------------------------
  // start application task
  //----------------------------------------------------------
  // Create all workers and pass stop tokens
  taskAppContext =
      std::move(std::thread(TaskAppContextFunc, std::ref(appContext), std::ref(appConfig), stop_src.get_token()));

  //----------------------------------------------------------
  // Main loop

  if (appConfig.hasTestConsole) {
    std::cout << "Press the h key to display the Console Menu..." << std::endl;
  }

  // Daemon main loop
  while (daemon.is_running()) {
    if (appConfig.hasTestConsole) {
      auto result = handle_console();
      switch (result) {
        case handleConsoleType::exit:
          daemon.set_state(app::Daemon::State::stop);
          break;
        case handleConsoleType::reload:
          daemon.set_state(app::Daemon::State::reload);
          break;
        case handleConsoleType::none:
        default:
          break;
      }
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  // set token to stop all worker
  stop_src.request_stop();

  spdlog::info("The daemon process is stopping");

  // wakeup all tasks
  {
    std::unique_lock lck(daemon_event.event_mutex);
    daemon_event.event_condition.notify_all();
  }

  spdlog::info("Waiting for the application task to complete");

  // Join threads
  taskAppContext.join();

  if (auto result = daemon.close_all(); result.has_value() && !result.value()) {
    spdlog::error("Error closing the daemon.");
    return EXIT_FAILURE;
  }

  spdlog::info("The daemon process ended successfully");

  return EXIT_SUCCESS;
}