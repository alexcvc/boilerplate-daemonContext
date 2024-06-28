/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

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
  /// constructor
  IAppContext() = default;

  /// destructor
  virtual ~IAppContext() = default;

  [[nodiscard]] virtual std::optional<bool> check_configuration(const app::DaemonConfig& config) = 0;

  [[nodiscard]] virtual std::optional<bool> prepare_before_to_start() = 0;


};

}   // namespace app