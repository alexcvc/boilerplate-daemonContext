/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

/*************************************************************************/ /**
 * \file
 * \brief  contains class with implementation application context functions.
 * \author
 * \date    2024-06-29
 * \ingroup
 *****************************************************************************/

#pragma once

#include <filesystem>
#include <chrono>

#include "appContextBase.hpp"

namespace app {

/**
 * @brief The AppContext class provides implementation application context functions.
 */
class AppContext : public IAppContext {
  // Private Variables
  std::filesystem::path m_configFile;   ///< The path of the configuration file
  std::filesystem::path m_pathConfig;   ///< The path of the configuration folder
  std::filesystem::path m_logFile;      ///< The path of the log file

 public:
  /// constructor
  AppContext() = default;

  /// destructor
  virtual ~AppContext() = default;

  /**
   * @brief Validates the configuration of the daemon.
   * @param config The configuration of the daemon to validate.
   * @return An optional boolean value. If the configuration is valid, the method returns an optional that contains the boolean value 'true'.
   *         If the configuration is not valid, the method returns an optional that contains the boolean value 'false'.
   *         If an error occurs during the validation process, the method returns an empty optional.
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
};

}   // namespace app