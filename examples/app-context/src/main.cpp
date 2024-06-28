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
#include <fmt/format.h>
#include <getopt.h>
#include <spdlog/spdlog.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "appContext.hpp"
#include "daemon.hpp"
#include "daemonConfig.hpp"
#include "version.hpp"

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

//----------------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Declarations
//----------------------------------------------------------------------------

/**
 * @brief Displays the version information of the program.
 * @param prog The name of the program.
 * @return None
 */
static void show_version(const char* prog) {
  auto verString = version::daemon_with_context::getVersion(true);
  // info
  std::cout << prog << " v." << verString << std::endl;
}

/**
 * Displays the help message for the program.
 * @param prog The name of the program.
 * @param var The character representing the option with an error. Defaults to 0.
 */
static void display_help(const char* prog, std::string_view errorOption = "") {
  if (!errorOption.empty()) {
    std::cerr << "Error in option: " << errorOption << "\n";
  }
  std::cout << "Usage: " << prog << " [OPTIONS]" << std::endl
            << "-D, --background         start as daemon" << std::endl
            << "-F, --foreground         start in foreground with test console" << std::endl
            << "-S, --cfgpath            path to folder with configuration files" << std::endl
            << "-x, --cfgfile            specified configuration file" << std::endl
            << "-P, --pidfile            create pid file" << std::endl
            << "-L, --logfile            specified log file" << std::endl
            << "-v, --version            version" << std::endl
            << "-h, --help               this message" << std::endl;
  std::cout << "Sample command lines:" << std::endl;
  std::cout << prog << " -F" << std::endl;
  std::cout << prog << " -D -P /var/run/some.pid" << std::endl;
  std::cout << prog << " -F -S /app/config" << std::endl;
  std::cout << prog << " -D -x /app/config/settings.xml -P /var/run/some.pid" << std::endl;

  if (!errorOption.empty()) {
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief Processes the command line options passed to the program.
 *
 * This function parses the command line options and sets the corresponding variables
 * based on the provided values. It uses getopt_long function to handle both short and
 * long options.
 *
 * @param argc The number of command line arguments.
 * @param argv The array of command line argument strings.
 * @param config
 */
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
        config.hasTestConsoleInForeground = false;
        break;

      case 'F':
        config.isDaemon = false;
        config.hasTestConsoleInForeground = true;
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

      case 'L':
        if (strlen(optarg)) {
          config.logFile.assign(optarg);
        } else {
          display_help(argv[0], std::to_string(var));
        }
      break;

      case 'x':
        if (strlen(optarg)) {
          config.configFile.assign(optarg);
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

/**
 * @file main.c
 * @brief This is the main entry point for the application.
 */
int main(int argc, char** argv) {
  // The Daemon class is a singleton to avoid be instantiated more than once
  app::Daemon& daemon = app::Daemon::instance();
  app::DaemonConfig config;
  app::AppContext appContext;

  //----------------------------------------------------------
  // parse parameters
  //----------------------------------------------------------
  process_command_line(argc, argv, config);

  //----------------------------------------------------------
  // parameters have been set
  //----------------------------------------------------------

  // Set the start all function
  daemon.set_start_function([]() {
    std::cout << "Start all function called." << std::endl;
    return true;
  });

  // Set the stop all function
  daemon.set_close_function([]() {
    std::cout << "Close all function called." << std::endl;
    return true;
  });

  // Set the reload function to be called in case of receiving a SIGHUP signal
  daemon.set_reload_function([]() {
    std::cout << "Reload function called." << std::endl;
    return true;
  });

  // Start all
  if (auto result = daemon.start_all(); result.has_value() && !result.value()) {
    std::cerr << "Error starting the daemon." << std::endl;
    return EXIT_FAILURE;
  }

  // Start all

  if (config.isDaemon) {
    if (auto result = daemon.make_daemon(config.pidFile); result.has_value() && !result.value()) {
      std::cerr << "Error starting the daemon." << std::endl;
      return EXIT_FAILURE;
    }
  }

  if (config.hasTestConsoleInForeground) {
    std::cout << "Press the h key to display the Console Menu..." << std::endl;
  }

  // Daemon main loop
  while (daemon.is_running()) {
    if (config.hasTestConsoleInForeground) {
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

  if (auto result = daemon.close_all(); result.has_value() && !result.value()) {
    std::cerr << "Error closing the daemon." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "The daemon process ended successfully" << std::endl;

  return EXIT_SUCCESS;
}