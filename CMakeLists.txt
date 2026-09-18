cmake_minimum_required(VERSION 4.3)

project(result CXX)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(result STATIC)
add_library(inco::result ALIAS result)

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

add_executable(result-demo)

target_sources(result-demo
        PUBLIC FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/main.cppm
)

target_link_libraries(result-demo PRIVATE inco::result)