/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2025 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

/**
 * @file
 * @brief Implementation of the AppContext class.
 *
 * This file contains the implementation of the AppContext class. The AppContext
 * class is responsible for managing the application context of the daemon. It
 * provides methods for validating the configuration, processing the reconfiguration,
 * starting the application context, restarting the application context, shutting
 * down the application context, and executing the application context.
 */

// clang-format off
#include <chrono>
#include <fstream>
#include <iostream>
#include "include/app_context.hpp"

#include <atomic>

#include "app_utilities.hpp"
// clang-format on

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Typedefs, enums, unions, variables
//----------------------------------------------------------------------------

// legacy indication about complete operation in threads
std::atomic_bool isRunningNow{true};

//----------------------------------------------------------------------------
// Declarations
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------{]}
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
 * @param isMandatory
 * @return The success status of the operation. Returns true if the error file was incremented successfully
 *         or already exists, false otherwise.
 ******************************************************************************/
bool app::AppContext::validate_path(const std::string& path, const std::string& desc, bool isMandatory) const {
  if (!path.empty()) {
    if (!std::filesystem::exists(path)) {
      std::cerr << desc << " \"" << path << "\" doesn't exist" << std::endl;
      return false;
    }
  } else if (isMandatory) {
    std::cerr << desc << " \"" << path << "\" is mandatory but not defined" << std::endl;
    return false;
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

  std::cout << "Application context: Validating the configuration" << std::endl;

  m_pathConfigFile = config.pathConfigFile;
  m_pathConfigFolder = config.pathConfigFolder;

  /*
   * Use the validatePath function to validate all paths.
   */
  if (!validate_path(m_pathConfigFolder, "Configuration Folder", false)) {
    errorCount++;
  }
  if (!validate_path(m_pathConfigFile, "XML Configuration", false)) {
    errorCount++;
  }

  // if there are errors, return false
  if (errorCount > 0) {
    return false;
  }

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
  std::cout << "Application context: Reconfiguring the application" << std::endl;
  // reload logging configuration from XML
  return true;
}

/*************************************************************************/ /**
 * @brief Process the restart of the application context.
 * @return An optional boolean value indicating if the process restart was successful.
 *         Returns std::nullopt if failed, otherwise returns true.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_restart() {
  std::cout << "Application context: Restarting the application" << std::endl;
  return true;
}

/*************************************************************************/ /**
 * @brief Process everything after USER1 signal
 * @return An optional boolean value indicating if the process was successful.
 *         Returns std::nullopt if failed, otherwise returns true.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_user1() {
  std::cout << "Application context: get and process the USER1 signal" << std::endl;
  return true;
}

/*************************************************************************/ /**
 * @brief Process everything after USER2 signal
 * @return An optional boolean value indicating if the process was successful.
 *         Returns std::nullopt if failed, otherwise returns true.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_user2() {
  std::cout << "Application context: get and process the USER2 signal" << std::endl;
  return true;
}

/*************************************************************************/ /**
 * \brief Process the start of the application context.
 *
 * This function sets up the necessary configurations and starts the stack
 * library with all subservices. It registers GOMSFE for all supported
 * versions, configures RFC1006 parameters from a file, initializes the
 * SCL configuration, adds GOOSE and R-GOOSE adapters/addresses,
 * sets up file services
 *
 * \return An optional boolean value indicating if the process start was successful.
 *         Returns std::nullopt if failed, otherwise returns true.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_start() {
  // lock stack context
  std::unique_lock lock(m_mutex);

  // check configuration and set settings.xml
  if (m_pathConfigFile.empty()) {
    m_pathConfigFile = m_pathConfigFolder / kDefaultConfigFile;
  }

  //-------------------------------------
  // Start event scheduler
  //-------------------------------------

  //-------------------------------------
  // Initialization of process image
  //-------------------------------------

  // ----------------------------------------------------------
  // Once at startup
  // ----------------------------------------------------------

  // ----------------------------------------------------------
  // start managing scheduled events, including control timeouts
  // ----------------------------------------------------------

  return true;
}

/*************************************************************************/ /**
 * @brief Performs a graceful shutdown of the application.
 * @return An optional boolean value indicating the success of the shutdown process.
 *         The optional value will be empty if the shutdown process encountered an error.
 ******************************************************************************/
std::optional<bool> app::AppContext::process_shutdown() {
  std::cout << "Application context: Shutting down the application" << std::endl;
  // lock stack context
  std::scoped_lock lock(m_mutex);

  // exit from the thread loops - legacy method.
  isRunningNow.store(false, std::memory_order_release);

  // ----------------------------------------------------------
  // terminate the scheduled events and destroy scheduler
  // ----------------------------------------------------------

  //------------------------------------
  // destroy task scheduler
  //------------------------------------

  return true;
}

/*************************************************************************/ /**
 * @brief processing the context.
 * @param min_duration minimum duration until next processing.
 * @return The earlier timeout until next process.
 ******************************************************************************/
std::chrono::milliseconds app::AppContext::process_executing(const std::chrono::milliseconds& min_duration) {
  return min_duration;
}
