cmake_minimum_required(VERSION 3.22)

# ========== Project Setup ==========

project(Specula VERSION 0.1.0 DESCRIPTION "A C++20 Physically Based Renderer" LANGUAGES CXX)

# ========== Global Options ==========

option(BUILD_SHARED_LIBS "Build shared libraries instead of static ones" ON)

# General build system options
option(ENABLE_CCACHE "Enable ccache for faster builds" ON)

# Global compiler options
option(ENABLE_PROFILING "Enable profiling instrumentation" OFF)
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

# Global sanitizer options
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_LSAN "Enable LeakSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# Controls for which targets to build
option(SPECULA_BUILD_TESTS "Build unit tests" ON)
option(SPECULA_BUILD_APP "Build the application" ON)
option(SPECULA_BUILD_DOCS "Build documentation" OFF)

# ========== Global Configuration =========

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(
    CMAKE_BUILD_TYPE
    "Release"
    CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
    FORCE
  )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Auto-detect the Git revision and use it as part of the project version
include(GetGitRevisionDescription)
git_describe(PROJECT_VERSION_COMMIT)

# Include Cmake Package Manager for dependency management
include(CPM)

# Setup CCache if enabled and available
if(ENABLE_CCACHE)
  find_package(sccache)
  if(sccache_FOUND)
    set(CMAKE_CXX_COMPILER_LAUNCHER_VERSION ${SCCACHE_VERSION})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_EXECUTABLE})
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_EXECUTABLE})
  else()
    find_package(ccache)
    if(ccache_FOUND)
      set(CMAKE_CXX_COMPILER_LAUNCHER_VERSION ${CCACHE_VERSION})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
    else()
      message(WARNING "Neither sccache nor ccache found. Build caching will be disabled.")
    endif()
  endif()
endif()

# ========== Interface Targets ==========

add_library(options INTERFACE)
target_compile_features(options INTERFACE cxx_std_20)

add_library(sanitizers INTERFACE)
set(ENABLED_SANITIZERS "")
function(enable_sanitizer flag name)
  if(${flag})
    set(ENABLED_SANITIZERS ${ENABLED_SANITIZERS} ${name} PARENT_SCOPE)
    target_compile_options(sanitizers INTERFACE -fsanitize=${name})
    target_link_options(sanitizers INTERFACE -fsanitize=${name})
  endif()
endfunction()
enable_sanitizer(ENABLE_ASAN "address")
enable_sanitizer(ENABLE_LSAN "leak")
enable_sanitizer(ENABLE_MSAN "memory")
enable_sanitizer(ENABLE_TSAN "thread")
enable_sanitizer(ENABLE_UBSAN "undefined")

include(Format)
include(Lint)

# ========== Build Targets ==========

add_subdirectory(tools)
add_subdirectory(src)

if(SPECULA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(SPECULA_BUILD_APP)
  add_subdirectory(app)
endif()

if(SPECULA_BUILD_DOCS)
  add_subdirectory(docs)
endif()

# ========== Configuration Summary ==========

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # cmake-format: off
  message(STATUS "====================[ ${PROJECT_NAME} v${PROJECT_VERSION} (${PROJECT_VERSION_COMMIT}) ]====================")
  message(STATUS "")
  message(STATUS "System:      ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
  message(STATUS "CMake:       ${CMAKE_VERSION}")
  message(STATUS "Generator:   ${CMAKE_GENERATOR}")
  if(CMAKE_CXX_COMPILER_LAUNCHER)
    message(STATUS "Launcher:    ${CMAKE_CXX_COMPILER_LAUNCHER} (${CMAKE_CXX_COMPILER_LAUNCHER_VERSION})")
  endif()
  message(STATUS "Compiler:    ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID}) v${CMAKE_CXX_COMPILER_VERSION}")
  message(STATUS "")
  message(STATUS "CMAKE_BUILD_TYPE:   ${CMAKE_BUILD_TYPE}")
  message(STATUS "BUILD_SHARED_LIBS:  ${BUILD_SHARED_LIBS}")
  message(STATUS "ENABLE_CCACHE:      ${ENABLE_CCACHE}")
  message(STATUS "")
  message(STATUS "SPECULA_BUILD_TESTS: ${SPECULA_BUILD_TESTS}")
  message(STATUS "SPECULA_BUILD_DOCS:  ${SPECULA_BUILD_DOCS}")
  message(STATUS "SPECULA_BUILD_APP:   ${SPECULA_BUILD_APP}")
  message(STATUS "")
  message(STATUS "ENABLE_PROFILING:   ${ENABLE_PROFILING}")
  message(STATUS "ENABLE_COVERAGE:    ${ENABLE_COVERAGE}")
  message(STATUS "")
  string(REPLACE ";" " " ENABLED_SANITIZERS_STRING "${ENABLED_SANITIZERS}")
  message(STATUS "SANITIZERS:         ${ENABLED_SANITIZERS_STRING}")
  message(STATUS "")
  message(STATUS "================================================================================")
  # cmake-format: on
endif()
