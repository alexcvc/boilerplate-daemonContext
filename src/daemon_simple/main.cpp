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
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "daemon.hpp"
#include "daemon_config.hpp"
#include "version.hpp"

//----------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Typedefs, enums, unions, variables
//----------------------------------------------------------------------------
enum class handleConsoleType {
  none,
  exit,
  abort,
  restart,
  reload,
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
  auto verString = version::daemon_simple::getVersion(true);
  // info
  std::cout << prog << " v." << verString << std::endl;
}

/**
 * Displays the help message for the program.
 * @param prog The name of the program.
 * @param var The character representing the option with an error. Defaults to 0.
 */
static void display_help(const char* prog, char errorOption = 0) {
  if (errorOption != 0) {
    std::cerr << "Error in option: " << errorOption << "\n";
  }
  std::cout << "Usage: " << prog << " [OPTIONS]" << std::endl
            << "-D, --background         start as daemon" << std::endl
            << "-T, --test               start in foreground with test console" << std::endl
            << "-S, --cfgpath            path to folder with configuration files" << std::endl
            << "-x, --cfgfile            specified configuration file" << std::endl
            << "-P, --pidfile            create pid file" << std::endl
            << "-v, --version            version" << std::endl
            << "-h, --help               this message" << std::endl;
  std::cout << "Sample command lines:" << std::endl;
  std::cout << prog << " -T" << std::endl;
  std::cout << prog << " -D -P /var/run/some.pid" << std::endl;
  std::cout << prog << " -T -S /app/config" << std::endl;
  std::cout << prog << " -D -x /app/config/settings.xml -P /var/run/some.pid" << std::endl;

  if (errorOption != 0) {
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
    static const char* short_options = "h?vDTP:S:x:L:";
    static const struct option long_options[] = {
        {"help", no_argument, 0, 0},
        {"version", no_argument, 0, 'v'},
        {"background", no_argument, 0, 'D'},
        {"test", no_argument, 0, 'T'},
        {"pidfile", required_argument, 0, 'P'},
        {"cfgpath", required_argument, 0, 'S'},
        {"cfgfile", required_argument, 0, 'x'},
        {0, 0, 0, 0},
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
          display_help(argv[0], var);
        }
        break;

      case 'S':
        if (strlen(optarg)) {
          config.pathConfigFolder.assign(optarg);
        } else {
          display_help(argv[0], var);
        }
        break;

      case 'x':
        if (strlen(optarg)) {
          config.pathConfigFile.assign(optarg);
        } else {
          display_help(argv[0], var);
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
    case 'a':
      return handleConsoleType::abort;
    case 'q':
      return handleConsoleType::exit;
    case 'R':
      return handleConsoleType::restart;
    case 'r':
      return handleConsoleType::reload;
    case '?':
    case 'h':
      fprintf(stderr, "Application test console:\n");
      fprintf(stderr, " r   -  reload debug settings\n");
      fprintf(stderr, " R   -  reload application\n");
      fprintf(stderr, " q   -  quit from application.\n");
      fprintf(stderr, " a   -  abort application.\n");
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

  //----------------------------------------------------------
  // parse parameters
  //----------------------------------------------------------
  process_command_line(argc, argv, config);

  //----------------------------------------------------------
  // parameters have been set
  //----------------------------------------------------------

  // Set the start-all function
  daemon.setStartFunction([]() {
    std::cout << "Start all function called." << std::endl;
    return true;
  });

  // Set the stop all function
  daemon.setCloseFunction([]() {
    std::cout << "Close all function called." << std::endl;
    return true;
  });

  // Set the reload function to be called in case of receiving a SIGHUP signal
  daemon.setReloadFunction([]() {
    std::cout << "Reload function called." << std::endl;
    return true;
  });

  // Start all
  if (auto result = daemon.startAll(); result.has_value() && !result.value()) {
    std::cerr << "Error starting the daemon." << std::endl;
    return EXIT_FAILURE;
  }

  // Start all

  if (config.isDaemon) {
    if (auto result = daemon.makeDaemon(config.pidFile); result.has_value() && !result.value()) {
      std::cerr << "Error starting the daemon." << std::endl;
      return EXIT_FAILURE;
    }
  }

  if (config.hasTestConsole) {
    std::cout << "Press the h key to display the Console Menu..." << std::endl;
  }

  // Daemon main loop
  while (daemon.isRunning()) {
    if (config.hasTestConsole) {
      auto result = handle_console();
      switch (result) {
        case handleConsoleType::exit:
          daemon.setState(app::Daemon::State::Stop);
          break;
        case handleConsoleType::abort:
          daemon.setState(app::Daemon::State::Stop);
          break;
        case handleConsoleType::restart:
          daemon.setState(app::Daemon::State::Reload);
          break;
        case handleConsoleType::reload:
          daemon.setState(app::Daemon::State::Reload);
          break;
        default:
          break;
      }
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  if (auto result = daemon.closeAll(); result.has_value() && !result.value()) {
    std::cerr << "Error closing the daemon." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "The daemon process ended successfully" << std::endl;

  return EXIT_SUCCESS;
}