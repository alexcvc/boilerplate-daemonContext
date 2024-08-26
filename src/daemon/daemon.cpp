/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

// clang-format off
#include "daemon.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <spdlog/spdlog.h>
#include <sys/stat.h>
// clang-format on

/**
 * @brief Constructor for the Daemon class.
 *
 * This constructor initializes the state of the daemon to 'start' and sets up signal handlers
 * for the 'ExitSignal', 'TerminateSignal', 'ReloadSignal', 'User1' and 'User2' signals.
 */
app::Daemon::Daemon() {
  m_state = State::start;
  signal(ExitSignal, Daemon::signal_handler);
  signal(TerminateSignal, Daemon::signal_handler);
  signal(ReloadSignal, Daemon::signal_handler);
  signal(UserSignal1, Daemon::signal_handler);
  signal(UserSignal2, Daemon::signal_handler);
}

/**
 * @brief Handles the interrupt signals received by the daemon.
 *
 * This function is responsible for handling interrupt signals received by the daemon.
 * It updates the state of the daemon based on the received signal.
 *
 * @param The signal number received.
 */
void app::Daemon::signal_handler(int signal) {
  spdlog::info("Interrupt signal number [{}] received.", signal);

  switch (signal) {
    case ExitSignal:
    case TerminateSignal: {
      Daemon::instance().m_state = State::stop;
      break;
    }
    case ReloadSignal: {
      Daemon::instance().m_state = State::reload;
      break;
    }
    case UserSignal1: {
      Daemon::instance().m_state = State::user1;
      break;
    }
    case UserSignal2: {
      Daemon::instance().m_state = State::user2;
      break;
    }
  }
}

/**
 * @brief Creates a child process.
 * @return The process ID of the child process. Returns -1 if the fork operation fails.
 */
pid_t app::Daemon::create_child_process() {
  pid_t pid = fork();  // create new process
  if (pid < 0) {
    // print error message and return -1 if fork failed
    perror("Can't fork first child");
  }
  // Quitting parent process
  else if (pid != 0) {
    // if pid != 0, it means we are in parent process, so we exit this process
    exit(0);
  }
  // return pid of newly created process in child and 0 in parent
  return pid;
}

/**
 * @brief Initializes the child process.
 *
 * This method is called to initialize the child process after it has been created.
 * It performs the following tasks:
 * 1. Creates a new session. The calling process becomes the leader of the new session.
 * 2. Changes the current working directory to root. This is to ensure that the daemon does not tie up the file system.
 *    The daemon should not prevent unmounting of file systems.
 * 3. Changes the file mode creation mask of the process. The umask function sets the process's file mode creation mask
 *    (umask) to mask & 0777 (only the file permission bits of mask are used), and returns the previous umask.
 * @return True if the child process is initialized successfully, false otherwise.
 * @see setsid()
 * @see chdir()
 * @see umask()
 */
bool app::Daemon::init_child_process() {
  // create a new session.
  if (setsid() < 0) {
    std::cerr << "failed to setsid" << std::system_error(errno, std::system_category()).what() << std::endl;
    return false;
  } else {
    try {
      // change the current working directory to root.
      std::filesystem::current_path("/");

      // The daemon will have the following permissions,
      // which are not going to make the security world very happy
      umask(0);

      // successfully initialized the child process
      return true;
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Filesystem exception caught by set current path '/': " << e.what() << std::endl;
    } catch (const std::system_error& e) {
      std::cerr << "System exception caught by set current path '/': " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Exception caught by set current path '/'" << std::endl;
    }
  }
  return false;
}

/**
 * @brief Writes the process ID to a file.
 *
 * This method writes the process ID to a specified file, allowing the process to be identified later.
 *
 * @param pid_file_name The name of the file to write the process ID to.
 * @return True if the process ID is written to the file successfully, false otherwise.
 */
bool app::Daemon::write_pid_to_file(const std::string& pid_file_name) {
  if (!pid_file_name.empty()) {
    std::fstream file_rback_stream(pid_file_name, std::ios::out);
    if (!file_rback_stream.is_open()) {
      std::cerr << "failed to open " << pid_file_name << std::endl;
      return false;
    }
    file_rback_stream << std::to_string(getpid());
    file_rback_stream.close();
  }
  return true;
}

/**
 * @brief Starts the daemon.
 *
 * This function starts the daemon by calling the handler function registered with the 'm_handlerBeforeToStart' member.
 * If the handler function returns 'true', the daemon is started by calling the 'make_daemon' function.
 *
 * @param The name of the file to write the process ID to.
 * @return 'true' if the daemon is started successfully, 'false' otherwise.
 */
std::optional<bool> app::Daemon::make_daemon(const std::string& pid_file_name) {
  pid_t pid = this->create_child_process();
  if (pid < 0) {
    return false;
  }

  if (!this->init_child_process()) {
    return false;
  }

  bool isWritten = this->write_pid_to_file(pid_file_name);
  if (!isWritten) {
    return false;
  }

  return true;
}
