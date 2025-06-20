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
 * @brief   Contains the configuration for a daemon process.
 * @ingroup
 */

#pragma once

#include <string>

namespace app {
/**
 * @struct DaemonConfig
 *
 * @brief The DaemonConfig struct represents the configuration for a daemon process.
 *
 * The DaemonConfig struct holds information such as the path of the PID file,
 * whether the process should run as a daemon, and whether there is a test console
 * running in the foreground.
 */
struct DaemonConfig {
  std::string pidFile;           ///< The path of the PID file
  bool isDaemon{false};          ///< Whether the process should run as a daemon
  bool hasTestConsole{false};    ///< Whether there is a test console running in the foreground
  std::string pathConfigFile;    ///< The path of the configuration file
  std::string pathConfigFolder;  ///< The path of the configuration folder
};
}  // namespace app
