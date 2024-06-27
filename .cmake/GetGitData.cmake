
SET(GIT_HASH "" CACHE STRING "The git revision")
SET(GIT_DATE "" CACHE STRING "The git revision date")

# git revision
if (GIT_HASH STREQUAL "")
   message(STATUS "Git hash not given. Looking it up")
   execute_process(
      COMMAND
      git rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE
      GIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

if (GIT_HASH STREQUAL "")
   message(WARNING "Failed to look up git hash. Using UTC time of cmake call")
   string(TIMESTAMP GIT_HASH "git_hash=?")
endif ()
message(STATUS "Git hash: ${GIT_HASH}")

## git date
if (GIT_DATE STREQUAL "")
   message(STATUS "Git date not given. Looking it up")
   execute_process(
      COMMAND
      git log -1 --format="%as" HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE
      GIT_DATE
      OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

if (GIT_DATE STREQUAL "")
   message(WARNING "Failed to look up git hash. Using UTC time of cmake call")
   string(TIMESTAMP GIT_DATE "git_date=?")
endif ()
message(STATUS "Git date: ${GIT_DATE}")
