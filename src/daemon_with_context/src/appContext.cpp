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

std::optional<bool> app::AppContext::validate_configuration(const app::DaemonConfig& config) {
  return std::nullopt;
}

std::optional<bool> app::AppContext::process_reconfigure() {
  return std::nullopt;
}

std::optional<bool> app::AppContext::process_start() {
  return std::nullopt;
}

std::optional<bool> app::AppContext::process_restart() {
  return std::nullopt;
}

std::optional<bool> app::AppContext::process_shutdown() {
  return std::nullopt;
}

std::chrono::milliseconds app::AppContext::process_executing(const std::chrono::milliseconds& min_duration) {
  return min_duration;
}
