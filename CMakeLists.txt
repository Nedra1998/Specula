cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
  Specula
  VERSION 0.2.0
  LANGUAGES CXX)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(SPECULA_MAIN_PROJECT TRUE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/find")

include(RequireOutOfSourceBuild)
include(SetDefaultBuildType)
include(FeatureSummary)
include(CMakeDependentOption)
include(CheckIPOSupported)
include(Format)
include(Analyze)
include(EnableExtraCompilerWarnings)
include(EnableProfiling)
include(EnableCodeCoverage)

set_default_build_type("Debug")

if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
endif()

check_ipo_supported(RESULT SPECULA_IPO OUTPUT output)
if(NOT SPECULA_IPO)
  message(WARNING "IPO s not supported: ${output}")
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

option(BUILD_TESTS "Build unit tests" ${SPECULA_MAIN_PROJECT})
if(BUILD_TESTS AND CMAKE_BUILD_TYPE MATCHES "Debug")
  option(CODE_COVERAGE "Enable code coverage reporting" ON)
else()
  set(CODE_COVERAGE OFF)
endif()

option(BUILD_DOCS "Build documentation" ${SPECULA_MAIN_PROJECT})
if(BUILD_DOCS AND CMAKE_BUILD_TYPE MATCHES "Debug")
  option(DOC_COVERAGE "Enable documentation coverage reporting" ON)
else()
  set(DOC_COVERAGE OFF)
endif()

option(ENABLE_FORMATTING "Enables automated formatters" ${SPECULA_MAIN_PROJECT})
option(ENABLE_ANALYSIS "Enable automated static analyzers" ${SPECULA_MAIN_PROJECT})

enable_testing()

include(FetchContent)
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY "https://github.com/CLIUtils/CLI11"
  GIT_TAG "v1.9.1")
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "7.1.3")
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY "https://github.com/gabime/spdlog"
  GIT_TAG "v1.8.5")
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY "https://github.com/catchorg/Catch2"
  GIT_TAG "v3.0.0-preview3")
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY "https://github.com/Neargye/magic_enum"
  GIT_TAG "v0.7.2")
FetchContent_MakeAvailable(magic_enum)

add_subdirectory(src)
add_subdirectory(test)

file(
  GLOB_RECURSE
  CMAKE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/docs/CMakeLists.txt")
format_sources(cmake-fmt TEST SOURCES ${CMAKE_SOURCES})
analyze_sources(cmake-analyze TEST SOURCES ${CMAKE_SOURCES})

if(SPECULA_MAIN_PROJECT)
  feature_summary(WHAT ALL QUIET_ON_EMPTY)
endif()
