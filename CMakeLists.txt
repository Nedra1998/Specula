cmake_minimum_required(VERSION 3.22)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# ===== Project =====

project(
  Specula
  VERSION 0.1.0
  DESCRIPTION "A C++20 Physically Based Renderer"
  LANGUAGES CXX)

# ===== Global Configuration =====
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GetGitRevisionDescription)
git_describe(PROJECT_VERSION_COMMIT)

# ===== Options =====
# Set the default build type to Release
option(CMAKE_BUILD_TYPE "Build type" Release)

# General compiling options
option(ENABLE_CCACHE "Enable CCache" ON)
option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" ON)

# Project target options
option(SPECULA_BUILD_TESTS "Enable compilation of tests" ON)
option(SPECULA_BUILD_DOCS "Enable compilation of documentation" ON)
option(SPECULA_BUILD_APP "Enable compilation of the application" ON)

# Debugging options
option(ENABLE_PROFILING "Enable Tracy profiling" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Static analysis options
option(ENABLE_IWYU "Enable Include-What-You-Use" OFF)
option(ENABLE_LWYU "Enable Link-What-You-Use" OFF)
option(ENABLE_CLANG_TIDY "Enable Clang-Tidy" OFF)
option(ENABLE_CPPCHECK "Enable CppCheck" OFF)

# Sanitizer options
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer" OFF)
option(ENABLE_LSAN "Enable LeakSanitizer" OFF)

# ===== Dependencies =====

include(CPM)

if(ENABLE_CCACHE)
  find_package(sccache)
  if(sccache_FOUND)
    set(CMAKE_CXX_COMPILER_LAUNCHER_VERSION "${SCCACHE_VERSION}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_EXECUTABLE}")
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_EXECUTABLE}")
  else()
    find_package(ccache)
    if(ccache_FOUND)
      set(CMAKE_CXX_COMPILER_LAUNCHER_VERSION "${CCACHE_VERSION}")
      set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
      set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    else()
      message(WARNING "CCache not found, build times may be longer")
    endif()
  endif()
endif()

set(ENABLED_LINTERS "")
if(ENABLE_IWYU)
  find_package(iwyu)
  if(iwyu_FOUND)
    list(APPEND ENABLED_LINTERS "IWYU")
  endif()
endif()

if(ENABLE_LWYU)
  list(APPEND ENABLED_LINTERS "LWYU")
endif()

if(ENABLE_CLANG_TIDY)
  find_package(clang-tidy)
  if(clang-tidy_FOUND)
    list(APPEND ENABLED_LINTERS "clang-tidy")
  endif()
endif()

if(ENABLE_CPPCHECK)
  find_package(cppcheck)
  if(cppcheck_FOUND)
    list(APPEND ENABLED_LINTERS "cppcheck")
  endif()
endif()

if(ENABLE_COVERAGE)
  include(EnableCodeCoverage)
endif()

# ===== Targets =====

set(ENABLED_SANITIZERS "")
add_library(sanitizers INTERFACE)
function(enable_sanitizer flag name)
  if(${flag})
    message(STATUS "Enabling ${name} sanitizer")
    set(ENABLED_SANITIZERS
        ${ENABLED_SANITIZERS} ${name}
        PARENT_SCOPE)
    target_compile_options(sanitizers INTERFACE -fsanitize=${name} -g)
    target_link_options(sanitizers INTERFACE -fsanitize=${name})
  endif()
endfunction()
enable_sanitizer(ENABLE_ASAN "address")
enable_sanitizer(ENABLE_TSAN "thread")
enable_sanitizer(ENABLE_UBSAN "undefined")
enable_sanitizer(ENABLE_MSAN "memory")
enable_sanitizer(ENABLE_LSAN "leak")

add_subdirectory(src)

if(SPECULA_BUILD_APP)
  add_subdirectory(app)
endif()

if(SPECULA_BUILD_DOCS)
  add_subdirectory(docs)
endif()

if(SPECULA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ===== Packaging =====

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "LuxAter")
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/\\\\.git/")

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_LOWER}-${PROJECT_VERSION}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME_LOWER}-${PROJECT_VERSION}-source")

include(CPack)

# ===== Summary =====

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
  string(REPLACE ";" " " ENABLED_LINTERS_STRING "${ENABLED_LINTERS}")
  message(STATUS "LINTERS:            ${ENABLED_LINTERS_STRING}")
  string(REPLACE ";" " " ENABLED_SANITIZERS_STRING "${ENABLED_SANITIZERS}")
  message(STATUS "SANITIZERS:         ${ENABLED_SANITIZERS_STRING}")
  message(STATUS "")
  message(STATUS "================================================================================")
  # cmake-format: on
endif()
