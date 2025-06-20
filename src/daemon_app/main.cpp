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

#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "daemon.hpp"
#include "daemon_config.hpp"
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
    "  -T, --test               start in foreground with test console\n",
    "  -S, --cfgpath            path to folder with configuration files\n",
    "  -x, --cfgfile            specified configuration file\n",
    "  -P, --pidfile            create pid file\n",
    "  -v, --version            version\n",
    "  -h, --help               this message\n"};

/**
 *  @brief The help options for the program.
 */
static const char* help_options = "h?vDTP:S:x:L:";
static const struct option long_options[] = {
    {"help", no_argument, nullptr, 0},
    {"version", no_argument, nullptr, 'v'},
    {"background", no_argument, nullptr, 'D'},
    {"test", no_argument, nullptr, 'T'},
    {"pidfile", required_argument, nullptr, 'P'},
    {"cfgpath", required_argument, nullptr, 'S'},
    {"cfgfile", required_argument, nullptr, 'x'},
    {nullptr, 0, nullptr, 0},
};

/**
 * @brief The sample command lines for the program.
 */
static const std::array<std::string_view, 4> SAMPLE_COMMANDS = {
    " -F\n", " -D -P /var/run/some.pid\n", " -T -S /app/config\n",
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
  auto verString = version::daemon_app::getVersion(true);
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

      case 'T':
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
      std::cout << " v." << version::daemon_app::getVersion(true) << std::endl;
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
void TaskAppContextFunc(app::DaemonConfig& daemon_config, std::stop_token token) {
  auto sooner = 1000ms;

  // Register a stop callback
  std::stop_callback stop_cb(token, [&]() {
    // Wake thread on stop request
    daemon_event.event_condition.notify_all();
  });

  std::cout << "application task started" << std::endl;

  while (true) {
    std::cout << "application task ticks " << sooner.count() << " ms" << std::endl;
    // observe serves states
    if (sooner.count() > 0) {
      // Start of a locked block
      std::unique_lock lck(daemon_event.event_mutex);
      daemon_event.event_condition.wait_for(lck, std::chrono::milliseconds(sooner));
    }

    //Stop if requested to stop
    if (token.stop_requested()) {
      std::cout << "got stop requested in the application task" << std::endl;
      break;
    }
  }  // End of while loop

  std::cout << "application task completed" << std::endl;
}

/*************************************************************************/ /**
 * @file main.c
 * @brief This is the main entry point for the application.
 *****************************************************************************/
int main(int argc, char** argv) {
  // The Daemon class is a singleton to avoid be instantiated more than once
  app::Daemon& daemon = app::Daemon::instance();
  app::DaemonConfig daemonConfig;  ///< The configuration of the daemon
  std::stop_source stop_src;       ///< stop token for the main loop
  std::jthread taskAppContext;
  //----------------------------------------------------------
  // parse parameters
  //----------------------------------------------------------
  process_command_line(argc, argv, daemonConfig);
  //----------------------------------------------------------
  // set in daemon all handlers
  //----------------------------------------------------------
  daemon.setStartFunction([&]() {
    std::cout << "Start all function called." << std::endl;
    return true;
  });
  daemon.setCloseFunction([&]() {
    std::cout << "Close all function called." << std::endl;
    return true;
  });
  daemon.setReloadFunction([&]() {
    std::cout << "Reload function called." << std::endl;
    return true;
  });
  daemon.setUser1Function([&]() {
    std::cout << "User1 function called." << std::endl;
    return true;
  });
  daemon.setUser2Function([&]() {
    std::cout << "User2 function called." << std::endl;
    return true;
  });
  //----------------------------------------------------------
  // Check integrity this configuration
  //----------------------------------------------------------
  //----------------------------------------------------------
  // Prepare application to start
  //----------------------------------------------------------
  //----------------------------------------------------------
  // Start all
  //----------------------------------------------------------
  if (auto result = daemon.startAll(); result.has_value() && !result.value()) {
    std::cerr << "Error starting the daemon." << std::endl;
    return EXIT_FAILURE;
  }
  if (daemonConfig.isDaemon) {
    if (auto result = daemon.makeDaemon(daemonConfig.pidFile); result.has_value() && !result.value()) {
      std::cerr << "Error starting the daemon." << std::endl;
      return EXIT_FAILURE;
    }
  }
  //----------------------------------------------------------
  // start application task
  //----------------------------------------------------------
  // Create all workers and pass stop tokens
  taskAppContext = std::move(std::jthread(TaskAppContextFunc, std::ref(daemonConfig), stop_src.get_token()));
  //----------------------------------------------------------
  // Main loop
  if (daemonConfig.hasTestConsole) {
    std::cout << "Press the h key to display the Console Menu..." << std::endl;
  }
  // Daemon main loop
  while (daemon.isRunning()) {
    if (daemonConfig.hasTestConsole) {
      auto result = handle_console();
      switch (result) {
        case handleConsoleType::exit:
          daemon.setState(app::Daemon::State::Stop);
          break;
        case handleConsoleType::reload:
          daemon.setState(app::Daemon::State::Reload);
          break;
        case handleConsoleType::none:
        default:
          break;
      }
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  std::cout << "The daemon process is stopping. Stop requested for an application task" << std::endl;
  // set token to stop all workers
  stop_src.request_stop();
  // wakeup all tasks
  {
    std::unique_lock lck(daemon_event.event_mutex);
    daemon_event.event_condition.notify_all();
  }
  std::cout << "Main waits for completion of application task" << std::endl;
  // Join threads
  taskAppContext.join();
  if (auto result = daemon.closeAll(); result.has_value() && !result.value()) {
    std::cerr << "Error closing the daemon." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "The daemon process ended successfully" << std::endl;

  return EXIT_SUCCESS;
}