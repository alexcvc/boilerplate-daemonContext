/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

#include "appContext.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include <thread>
#include <filesystem>
#include <spdlog/spdlog.h>

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Typedefs, enums, unions, variables
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Declarations
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Definitions
//----------------------------------------------------------------------------

/*************************************************************************/ /**
 * @brief Validates the path of the error file.
 *
 * This method checks if the specified error file exists. If the file does not exist,
 * it will be created and the error message will be written to it. If the file exists,
 * the method will return true to indicate that the error file was incremented successfully.
 *
 * @param path The path of the error file.
 * @param desc The error message to be written to the file.
 * @return The success status of the operation. Returns true if the error file was incremented successfully
 *         or already exists, false otherwise.
 ******************************************************************************/
bool app::AppContext::validate_path(const std::string& path, const std::string& desc) const {
  spdlog::info("Validating path: {}", path);

  if (!path.empty()) {
    if (!std::filesystem::exists(path)) {
      spdlog::error("{} \"{}\" doesn't exist", desc, path);
      return false;
    }
  }
  return true;
}

/*************************************************************************/ /**
 * @brief Validates the configuration of the daemon.
 * @param config The configuration of the daemon to validate.
 * @return An optional boolean value. If the configuration is valid, the method
 * returns an optional that contains the boolean value 'true'.
 * If the configuration is not valid, the method returns an optional that contains
 * the boolean value 'false'.
 * If an error occurs during the validation process, the method returns an empty optional.
 ******************************************************************************/
std::optional<bool> app::AppContext::validate_configuration(const app::DaemonConfig& config) {
  int errorCount{0};

  spdlog::info("Validating the configuration");

  m_pathConfigFile = config.pathConfigFile;
  m_pathConfigFolder = config.pathConfigFolder;
  m_pathLogFile = config.logFile;

  /*
   * Use the validatePath function to validate all paths.
   */
  if (!validate_path(m_pathConfigFolder, "Configuration Folder")) {
    errorCount++;
  }

  if (!validate_path(m_pathConfigFile, "Configuration file")) {
    errorCount++;
  }

  if (!validate_path(m_pathLogFile, "Log File")) {
    errorCount++;
  }

  if (errorCount > 0)
    return false;

  return true;
}

/*************************************************************************/ /**
* @brief Processes the reconfiguration of the daemon.
*
* This method is responsible for handling the reconfiguration of the
* application. It performs all necessary tasks related to the reconfiguration
* process. The method returns an optional boolean value to indicate the success
* or failure of the reconfiguration.
*
* @return An optional boolean value. If the reconfiguration is successful,
* the method returns an optional that contains the boolean value 'true'.
* If the reconfiguration is not successful, the method returns an optional that
* contains the boolean value 'false'. If an error occurs during the reconfiguration
* process, the method returns an empty optional.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_reconfigure() {
  spdlog::info("Reconfiguring the application");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}

/*************************************************************************/ /**
 * @brief Process the restart of the application context.
 * @return An optional boolean value indicating if the process restart was successful.
 *         Returns std::nullopt if failed, otherwise returns true.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_restart() {
  spdlog::info("Restarting the application");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}

/*************************************************************************/ /**
 * @brief Process everything after USER1 signal
 * @return An optional boolean value indicating if the process was successful.
 *         Returns std::nullopt if failed, otherwise returns true.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_user1() {
  spdlog::info("Processing USER1 signal");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}

/*************************************************************************/ /**
 * @brief Process everything after USER2 signal
 * @return An optional boolean value indicating if the process was successful.
 *         Returns std::nullopt if failed, otherwise returns true.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_user2() {
  spdlog::info("Processing USER2 signal");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}

/*************************************************************************/ /**
 * \brief Process the start of the application context.
 * \return An optional boolean value indicating if the process start was successful.
 *         Returns std::nullopt if failed, otherwise returns true.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_start() {
  spdlog::info("Starting the application");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}

/*************************************************************************/ /**
 * @brief Performs a graceful shutdown of the application.
 * @return An optional boolean value indicating the success of the shutdown process.
 *         The optional value will be empty if the shutdown process encountered an error.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_shutdown() {
  spdlog::info("Shutting down the application");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}

/*************************************************************************/ /**
 * @brief processing the context.
 * @param min_duration minimum duration until next processing.
 * @return The earlier timeout until next process.
 ******************************************************************************/
std::chrono::milliseconds app::AppContext::process_executing(const std::chrono::milliseconds& min_duration) {
  spdlog::info("Processing the context. Minimal duration: {} ms", min_duration.count());
  return min_duration > std::chrono::milliseconds(5000) ? std::chrono::milliseconds(1000)
                                                        : min_duration + std::chrono::milliseconds(1000);
}
