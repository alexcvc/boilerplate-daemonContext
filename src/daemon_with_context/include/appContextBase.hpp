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
#include <optional>

#include "daemonConfig.hpp"

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
namespace app {
/**
 * @brief The IAppContext class provides an abstract base class for application context.
 */
class IAppContext {
  // Private Variables

 public:
  /**
   * @brief constructor
   */
  IAppContext() = default;

  /**
   * @brief destructor
   */
  virtual ~IAppContext() = default;

  /**
   * @brief validate the configuration of the application before starting.
   * @param config The configuration to check.
   * @return std::optional<bool> true if successful, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] virtual std::optional<bool> validate_configuration(const app::DaemonConfig& config) = 0;

  /**
   * @brief Process everything before reconfiguring the application
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] virtual std::optional<bool> process_reconfigure() = 0;

  /**
   * @brief Process everything before starting the application
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] virtual std::optional<bool> process_start() = 0;
  [[nodiscard]] static std::optional<bool> process_start(IAppContext& self) {
    return self.process_start();
  }

  /**
   * @brief Process everything before restarting the application
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] virtual std::optional<bool> process_restart() = 0;
  [[nodiscard]] static std::optional<bool> process_restart(IAppContext& self) {
    return self.process_restart();
  }

  /**
   * @brief Process everything after USER1 signal
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] virtual std::optional<bool> process_user1() = 0;
  [[nodiscard]] static std::optional<bool> process_user1(IAppContext& self) {
    return self.process_user1();
  }

  /**
   * @brief Process everything after USER2 signal
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] virtual std::optional<bool> process_user2() = 0;
  [[nodiscard]] static std::optional<bool> process_user2(IAppContext& self) {
    return self.process_user2();
  }

  /**
   * @brief Process everything before leaving the application
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] virtual std::optional<bool> process_shutdown() = 0;
  [[nodiscard]] static std::optional<bool> process_shutdown(IAppContext& self) {
    return self.process_shutdown();
  }

  /**
   * @brief processing the context.
   * @param min_duration minimum duration until next processing.
   * @return The earlier timeout until next process.
   */
  [[nodiscard]] virtual std::chrono::milliseconds process_executing(const std::chrono::milliseconds& min_duration) = 0;
  [[nodiscard]] static std::chrono::milliseconds process_executing(IAppContext& self,
                                                                   const std::chrono::milliseconds& min_duration) {
    return self.process_executing(min_duration);
  }
};

}  // namespace app
