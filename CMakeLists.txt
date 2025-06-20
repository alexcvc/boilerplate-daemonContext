cmake_minimum_required(VERSION 3.5)

# Set the PROJECT_NAME, PROJECT_VERSION as well as other variable
project(boilerplate_daemonCpp
   VERSION 1.0.0
   DESCRIPTION "C++ daemon boilerplate"
   LANGUAGES CXX C
)

### set readable summary for this version
set(PROJECT_VERSION_DESCRIPTION "Test and probes upgrade to ed2-1 and new stack  v12.2.0")

### Options
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" OFF)
option(BUILD_DOC "Build documentation" OFF)
option(BUILD_TEST "Build test-applications" OFF)

# Defines the CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(CTest)

# Control where libraries and executables are placed during the build (development).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

#######################################################
### Compiler
#######################################################

# Disable C and C++ compiler extensions.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add packages
find_package(Threads REQUIRED)

# Build debug-configuration by default
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif ()

# Strip everything
add_link_options($<$<CONFIG:RELEASE>:-s>)

#######################################################
### git version
#######################################################
# version handling
execute_process(
   COMMAND
   git rev-parse --short HEAD
   OUTPUT_VARIABLE
   GIT_HASH
   OUTPUT_STRIP_TRAILING_WHITESPACE)

#######################################################
### Version handling
#######################################################

# Make a version file containing the hash and date from git.
include(${CMAKE_CURRENT_SOURCE_DIR}/.cmake/GetGitData.cmake)

# Make a version file containing the hash and date from git.
configure_file("${CMAKE_SOURCE_DIR}/.cmake/version.cpp.in" "${CMAKE_BINARY_DIR}/version.cpp")
configure_file("${CMAKE_SOURCE_DIR}/.cmake/version.hpp.in" "${CMAKE_BINARY_DIR}/version.hpp")

#######################################################
### Add subdirectories
#######################################################

### submodules
#set(SPDLOG_FMT_EXTERNAL_HO TRUE)
#set(SPDLOG_BUILD_SHARED TRUE)

#add_subdirectory(submodules)

### library and include files.
add_subdirectory(lib)

### application.
add_subdirectory(src)

# Check if we need to build the tests
if (BUILD_TEST)
  add_subdirectory(test)
endif ()

# Add targets related to doxygen documentation generation
if (BUILD_DOC)
  add_subdirectory(docs)
endif ()
