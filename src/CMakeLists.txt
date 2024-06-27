### Set project name
set(TargetName boilerplate)

# Set the PROJECT_NAME, PROJECT_VERSION as well as other variable
project(${TargetName}
        VERSION 1.0.0
        DESCRIPTION "Boilerplate for base C++ application"
        LANGUAGES CXX C
)

### set readable summary for this version
set(PROJECT_VERSION_DESCRIPTION "Boilerplate C++ application for base project")

### List of CPP (source) library files.
set(${TargetName}_SRC
        "src/daemon.cpp"
        "src/main.cpp"
)

### List of HPP (header) library files.
set(${TargetName}_HDR
        "include/daemon.hpp"
)

### add executable
add_executable(${TargetName}
        ${CMAKE_BINARY_DIR}/version.cpp
        ${CMAKE_BINARY_DIR}/version.hpp
        ${${TargetName}_SRC}
        ${${TargetName}_HDR}
)

target_include_directories(${TargetName} PRIVATE
        ${CMAKE_BINARY_DIR}
        "include"
)

target_link_libraries(${TargetName} PRIVATE
        fmt
)

# post build copy optional
if ((NOT ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}) AND (IS_DIRECTORY ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}))
    message(STATUS "Target ${TargetName} will be installed in ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}")
    # Copy target file to another location in a post build step in
    add_custom_command(TARGET ${TargetName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TargetName}> ${CMAKE_POSTBUILD_OUTPUT_DIRECTORY}
    )
endif ()

install(TARGETS ${TargetName} DESTINATION bin)
