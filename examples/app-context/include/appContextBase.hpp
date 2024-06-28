/* SPDX-License-Identifier: MIT */
//
// Copyright (c) 2024 Alexander Sacharov <a.sacharov@gmx.de>
//               All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//

#pragma once

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
  ~IAppContext() = default;

  /// Copy constructor
  IAppContext(const IAppContext& other) = delete;

  /// Copy assignment operator
  IAppContext& operator=(const IAppContext& other) = delete;

  /// alternatively, reuse destructor and copy ctor
  IAppContext& operator=(IAppContext other) = delete;
};

}   // namespace app