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

/**
 * @brief The options for the program.
 */
static const std::array<std::string_view, 8> OPTIONS = {
    "  -D, --background         start as daemon\n",
    "  -F, --foreground         start in foreground with test console\n",
    "  -S, --cfgpath            path to folder with configuration files\n",
    "  -x, --cfgfile            specified configuration file\n",
    "  -P, --pidfile            create pid file\n",
    "  -L, --logfile            specified log file\n",
    "  -v, --version            version\n",
    "  -h, --help               this message\n"};

/**
 *  @brief The help options for the program.
 */
static const char* help_options = "h?vDFP:S:x:L:";
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

/**
 * @brief The sample command lines for the program.
 */
static const std::array<std::string_view, 4> SAMPLE_COMMANDS = {
    " -F\n", " -D -P /var/run/some.pid\n", " -F -S /app/config\n",
    " -D -x /app/config/settings.xml -P /var/run/some.pid\n"};

//----------------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Declarations
//----------------------------------------------------------------------------

/*************************************************************************/ /**
 * @brief Prints the sample command lines for the program.
 * @param programName The name of the program.
 * @return None
 *****************************************************************************/
static void print_sample_commands(const char* programName) {
  std::cout << "\nSample command lines:" << std::endl;
  for (const auto& cmd : SAMPLE_COMMANDS) {
    std::cout << programName << cmd;
  }
}

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
 * @param programName The name of the program.
 * @param var The character representing the option with an error. Defaults to 0.
 *****************************************************************************/
static void display_help(const char* programName, std::string_view errorOption = "") {
  if (!errorOption.empty()) {
    std::cerr << "Error in option: " << errorOption << "\n";
  }
  std::cout << "\nUsage: " << programName << " [OPTIONS]\n" << std::endl;
  for (const auto& option : OPTIONS) {
    std::cout << option;
  }
  // output sample commands
  print_sample_commands(programName);

  if (!errorOption.empty()) {
    exit(EXIT_FAILURE);
  }
}

/*************************************************************************/ /**
 * @brief Handles the argument for a given option.
 * @param option The option for which the argument is provided.
 * @param argument The argument provided for the option.
 * @param argv0 The name of the program.
 * @return None
 ****************************************************************************/
void handle_option_argument(const char* option, const char* argument, const char* argv0) {
  if (!strlen(argument)) {
    std::cerr << "Missing " << option << " argument for option\n";
    display_help(argv0);
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
  int option_index = 0;
  for (;;) {
    int current_option = getopt_long(argc, argv, help_options, long_options, &option_index);
    if (current_option == -1) {
      break;
    }

    switch (current_option) {
      case 0:
        display_help(argv[0]);
        break;

      case 'h':
      case '?':
        display_help(argv[0]);
        exit(EXIT_SUCCESS);

      case 'v':
        show_version(argv[0]);
        exit(EXIT_SUCCESS);

      case 'D':
        config.isDaemon = true;
        config.hasTestConsole = false;
        break;

      case 'F':
        config.isDaemon = false;
        config.hasTestConsole = true;
        break;

      case 'P':
        handle_option_argument("pid-file", optarg, argv[0]);
        config.pidFile.assign(optarg);
        break;

      case 'S':
        handle_option_argument("configuration path", optarg, argv[0]);
        config.pathConfigFolder.assign(optarg);
        break;

      case 'x':
        handle_option_argument("configuration file", optarg, argv[0]);
        config.pathConfigFile.assign(optarg);
        break;

      case 'L':
        handle_option_argument("log file", optarg, argv[0]);
        config.logFile.assign(optarg);
        break;

      default:
        std::cerr << "Unknown option: " << std::to_string(current_option) << std::endl;
        display_help(argv[0]);
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