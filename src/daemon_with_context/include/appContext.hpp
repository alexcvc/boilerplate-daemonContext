/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

/**
* \file
 * \brief   contains the application context class
 * \ingroup Daemon with Application Context
 */

#pragma once

#include <chrono>
#include <filesystem>
#include <mutex>

#include "appContextBase.hpp"

namespace app {

/**
 * @brief The AppContext class provides implementation application context functions.
 */
class AppContext : public IAppContext {
  // Private Variables
  std::filesystem::path m_pathConfigFile;    ///< The path of the configuration file
  std::filesystem::path m_pathConfigFolder;  ///< The path of the configuration folder
  std::filesystem::path m_pathLogFile;       ///< The path of the log file

 public:
  /// constructor
  AppContext() = default;

  /// destructor
  virtual ~AppContext() = default;

  /**
   * @brief Validates the configuration of the daemon.
   * @param config The configuration of the daemon to validate.
   * @return An optional boolean value.
   */
  [[nodiscard]] std::optional<bool> validate_configuration(const app::DaemonConfig& config) override;

  /**
   * @brief Process everything before reconfiguring the application.
   * @return std::optional<bool> true if the process completes successfully, otherwise false.
   */
  [[nodiscard]] std::optional<bool> process_reconfigure() override;

  /**
   * @brief Process everything before starting the application.
   * @return std::optional<bool> - true if the process_start is successful, false otherwise.
   * If the method is not implemented, the return value is not defined.
   */
  [[nodiscard]] std::optional<bool> process_start() override;

  /**
   * @brief Process everything before restarting the application
   * @return std::optional<bool> - true if the restart process was successful, false otherwise.
   * @note This function is pure virtual and must be implemented by the derived class.
   */
  [[nodiscard]] std::optional<bool> process_restart() override;

  /**
   * @brief Process everything after USER1 signal
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] std::optional<bool> process_user1() override;

  /**
   * @brief Process everything after USER2 signal
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] std::optional<bool> process_user2() override;

  /**
   * @brief Performs a graceful shutdown of the application.
   * @return An optional boolean value indicating the success of the shutdown process.
   *         The optional value will be empty if the shutdown process encountered an error.
   */
  [[nodiscard]] std::optional<bool> process_shutdown() override;

  /**
   * @brief processing the context.
   * @param min_duration minimum duration until next processing.
   * @return The earlier timeout until next process.
   */
  [[nodiscard]] std::chrono::milliseconds process_executing(const std::chrono::milliseconds& min_duration) override;

  /**
   * @brief Set the path of the configuration file.
   * @param path The path of the configuration file.
   * @return true if the path exists or empty, otherwise false.
   */
  [[nodiscard]] bool validate_path(const std::string& path, const std::string& desc) const;
};

}  // namespace app
