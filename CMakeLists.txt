cmake_minimum_required(VERSION 4.3)

# Stand up a project scope only when configured on its own. In-tree we inherit the superbuild's standard + toolchain.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(result CXX)
    set(CMAKE_CXX_STANDARD 26)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

add_library(result STATIC)
add_library(result::result ALIAS result)

# PUBLIC file set. the module BMI must be visible to consumers so they can import result
target_sources(result
        PUBLIC FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/error.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/result.cppm
)

# TRY macros
target_include_directories(result PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(result PUBLIC cxx_std_26)
