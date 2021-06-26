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

# find_program(CCACHE_PROGRAM ccache) if(CCACHE_PROGRAM)
# set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
# set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") endif()

option(BUILD_TESTS "Build unit tests" ${SPECULA_MAIN_PROJECT})
add_feature_info(Tests BUILD_TESTS "Includes testing libraries and commands")
if(BUILD_TESTS AND CMAKE_BUILD_TYPE MATCHES "Debug")
  option(CODE_COVERAGE "Enable code coverage reporting" OFF)
else()
  set(CODE_COVERAGE OFF)
endif()

option(BUILD_DOCS "Build documentation" ${SPECULA_MAIN_PROJECT})
add_feature_info(Docs BUILD_DOCS
                 "Includes documentation and reference generations")
if(BUILD_DOCS AND CMAKE_BUILD_TYPE MATCHES "Debug")
  option(DOC_COVERAGE "Enable documentation coverage reporting" OFF)
else()
  set(DOC_COVERAGE OFF)
endif()

option(ENABLE_FORMATTING "Enables automated formatters" ${SPECULA_MAIN_PROJECT})
add_feature_info(Formatting ENABLE_FORMATTING
                 "Enables automated formatting checks")
option(ENABLE_LINTING "Enable automated static analyzers"
       ${SPECULA_MAIN_PROJECT})
add_feature_info(Linting ENABLE_LINTING
                 "Enables automated linting and code checks")

set(HIPSYCL_TARGETS)
option(ENABLE_ROCM "Enable ROCm AMD GPU acceleration" OFF)
add_feature_info(ROCm ENABLE_ROCM "Adds support for AMD ROCm parallelization")
if(ENABLE_ROCM)
  option(TARGET_VEGA "Target VEGA AMD GPUs" TRUE)
  option(TARGET_RDNA1 "Target RDNA 1 AMD GPUs" TRUE)

  set(HIP_TARGETS)
  if(TARGET_VEGA)
    list(APPEND HIP_TARGETS "gfx900" "gfx906")
  endif()
  if(TARGET_RDNA1)
    list(APPEND HIP_TARGETS "gfx1010" "gfx1012")
  endif()
  list(JOIN HIP_TARGETS "," HIP_TARGET)
  list(APPEND HIPSYCL_TARGETS "hip:${HIP_TARGET}")
endif()

option(ENABLE_CUDA "Enable Cuda Nvidia GPU acceleration" OFF)
add_feature_info(Cuda ENABLE_CUDA "Adds support for Nvida Cuda parallelization")
if(ENABLE_CUDA)
  option(TARGET_VOLTA "Target VOLTA Nvidia GPUs" TRUE)
  option(TARGET_PASCAL "Target VOLTA Nvidia GPUs" TRUE)

  set(CUDA_TARGETS)
  if(TARGET_VOLTA)
    list(APPEND CUDA_TARGETS "sm_70")
  endif()
  if(TARGET_PASCAL)
    list(APPEND CUDA_TARGETS "sm_60")
  endif()
  list(JOIN CUDA_TARGETS "," CUDA_TARGET)
  list(APPEND HIPSYCL_TARGETS "cuda:${CUDA_TARGET}")
endif()

option(ENABLE_OMP "Enable OpenMP CPU acceleration" ${SPECULA_MAIN_PROJECT})
add_feature_info(OpenMP ENABLE_OMP "Adds support for OpenMP parallelization")
if(ENABLE_OMP)
  list(APPEND HIPSYCL_TARGETS "omp")
endif()

list(REMOVE_DUPLICATES HIPSYCL_TARGETS)
find_package(hipSYCL CONFIG REQUIRED)
set_package_properties(
  hipSYCL PROPERTIES
  URL "https://github.com/illuhad/hipSYCL"
  DESCRIPTION "Multi-backend implementation of SYCL for CPUs and GPUs"
  TYPE REQUIRED
  PURPOSE "Add support for cross-platform C++ parallel programming with SYCL")

enable_testing()

include(FetchContent)
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY "https://github.com/CLIUtils/CLI11"
  GIT_TAG "v1.9.1"
  GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "7.1.3"
  GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY "https://github.com/gabime/spdlog"
  GIT_TAG "v1.8.5"
  GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(spdlog)

# FetchContent_Declare(
#   Catch2
#   GIT_REPOSITORY "https://github.com/catchorg/Catch2"
#   GIT_TAG "v3.0.0-preview3"
#   GIT_PROGRESS TRUE)
# FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY "https://github.com/Neargye/magic_enum"
  GIT_TAG "v0.7.2"
  GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(magic_enum)

add_subdirectory(src)
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

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
