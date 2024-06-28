/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

#pragma once

#include "appContextBase.hpp"
namespace app {

/**
 * @brief The AppContext class provides an class for application context.
 */
class AppContext : public IAppContext {
  // Private Variables

 public:
  /// constructor
  AppContext() = default;

  /// destructor
  ~AppContext() = default;

  /// Copy constructor
  AppContext(const AppContext& other) = delete;

  /// Copy assignment operator
  AppContext& operator=(const AppContext& other) = delete;

  /// alternatively, reuse destructor and copy ctor
  AppContext& operator=(AppContext other) = delete;
};
}   // namespace app