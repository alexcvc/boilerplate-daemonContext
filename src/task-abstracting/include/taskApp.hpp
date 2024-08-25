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
 * \brief   contains the daemon class
 * \author  Alexander Sacharov
 * \date    2024-06-26
 * \ingroup
 *****************************************************************************/

#pragma once

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include <thread>
#include <stop_token>
#include "appContext.hpp"

//----------------------------------------------------------------------------
// Declarations
//----------------------------------------------------------------------------
namespace app {

class TaskApp {
public:
  TaskApp(AppContext& context, const AppConfig& config);
  ~TaskApp();

  void start();
  void stop();

private:
  void taskAppContextFunc();

  AppContext& appContext;
  const AppConfig& appConfig;
  std::jthread taskAppContextThread;
  std::stop_source stopSource;
};

}  // namespace app