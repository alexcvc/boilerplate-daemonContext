cmake_minimum_required(VERSION 3.5)

### Set project name
set(TargetName daemon)

# Set the PROJECT_NAME, PROJECT_VERSION as well as other variable
project(${TargetName}
   VERSION 1.0.0
   DESCRIPTION "C++ daemon library with application context"
   LANGUAGES CXX C
)

### set readable summary for this version
set(PROJECT_VERSION_DESCRIPTION "Example, how to use application context in daemon application")

find_package(Threads REQUIRED)

### List of CPP (source) library files.
set(${TargetName}_SRC
   "daemon.cpp"
)

### List of HPP (header) library files.
set(${TargetName}_HDR
   "include/daemon.hpp"
   "include/daemon_config.hpp"
)

### add executable
add_library(${TargetName} STATIC ${${TargetName}_SRC}
   ${${TargetName}_HDR}
)

target_include_directories(${TargetName} PRIVATE
   "include"
)

install(TARGETS ${TargetName} DESTINATION lib)
