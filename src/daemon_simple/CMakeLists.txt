cmake_minimum_required(VERSION 3.5)

### Set project name
set(TargetName daemon_simple)

# Set the PROJECT_NAME, PROJECT_VERSION as well as other variable
project(${TargetName}
   VERSION 1.0.0
   DESCRIPTION "C++ daemon simple boilerplate"
   LANGUAGES CXX C
)

### set readable summary for this version
set(PROJECT_VERSION_DESCRIPTION "Example, how to use daemon boilerplate")

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

### List of CPP (source) library files.
set(${TargetName}_SRC
   "src/daemon.cpp"
   "src/main.cpp"
)

### List of HPP (header) library files.
set(${TargetName}_HDR
   "include/daemon.hpp"
   "include/daemonConfig.hpp"
)

# Make a version file containing the hash and date from git.
configure_file("${CMAKE_SOURCE_DIR}/.cmake/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")
configure_file("${CMAKE_SOURCE_DIR}/.cmake/version.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.hpp")

### add executable
add_executable(${TargetName}
   ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
   ${${TargetName}_SRC}
   ${${TargetName}_HDR}
)

target_include_directories(${TargetName} PRIVATE
   ${SPDLOG_HEADERS_DIR}
   ${FMT_HEADERS_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   "include"
)

find_package(fmt)
target_link_libraries(${TargetName} PRIVATE fmt::fmt-header-only spdlog::spdlog_header_only Threads::Threads)

# post build copy optional
if ((NOT ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}) AND (IS_DIRECTORY ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}))
  message(STATUS "Target ${TargetName} will be installed in ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}")
  # Copy target file to another location in a post build step in
  add_custom_command(TARGET ${TargetName} POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TargetName}> ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}
  )
endif ()

install(TARGETS ${TargetName} DESTINATION bin)
