#include "TaskApp.h"

//----------------------------------------------------------
// start application task
//----------------------------------------------------------
// Create task app instance and manage its lifecycle
TaskApp taskApp(appContext, appConfig);
taskApp.start();

// Perform other operations...

// When you need to stop the task
taskApp.stop();