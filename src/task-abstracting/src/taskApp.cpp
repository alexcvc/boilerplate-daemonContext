#include "TaskApp.h"

TaskApp::TaskApp(AppContext& context, const AppConfig& config)
    : appContext(context), appConfig(config) {
  // Constructor logic here, if needed
}

TaskApp::~TaskApp() {
  stop();
}

void TaskApp::start() {
  stopSource = std::stop_source();
  taskAppContextThread = std::jthread(&TaskApp::taskAppContextFunc, this);
}

void TaskApp::stop() {
  stopSource.request_stop();
  if (taskAppContextThread.joinable()) {
    taskAppContextThread.join();
  }
}

void TaskApp::taskAppContextFunc() {
  // TaskAppContextFunc logic here
  TaskAppContextFunc(appContext, appConfig, stopSource.get_token());
}