cmake_minimum_required(VERSION 3.5)

### Set project name
set(TargetName daemon_app)

# Set the PROJECT_NAME, PROJECT_VERSION as well as other variable
project(${TargetName}
   VERSION 1.0.0
   DESCRIPTION "C++ daemon example"
   LANGUAGES CXX C
)

### set readable summary for this version
set(PROJECT_VERSION_DESCRIPTION "Example, how to use daemon in C++ multi-threading application")

find_package(Threads REQUIRED)

### List of CPP (source) library files.
file(GLOB_RECURSE ${TargetName}_SRC *.c*)

### List of HPP (header) library files.
file(GLOB_RECURSE ${TargetName}_HDR *.h*)

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
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_SOURCE_DIR}/lib/include
   "include"
)

target_link_libraries(${TargetName} PRIVATE daemon Threads::Threads)

# post build copy optional
if ((NOT ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}) AND (IS_DIRECTORY ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}))
  message(STATUS "Target ${TargetName} will be installed in ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}")
  # Copy target file to another location in a post build step in
  add_custom_command(TARGET ${TargetName} POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TargetName}> ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}
  )
endif ()

install(TARGETS ${TargetName} DESTINATION bin)
