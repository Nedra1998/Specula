# ========== Options ==========
# ========== Dependencies ==========

CPMAddPackage("gh:fmtlib/fmt#12.1.0")
CPMAddPackage("gh:jeremy-rifkin/cpptrace@1.0.4")
CPMAddPackage(URI "gh:gabime/spdlog@1.17.0" OPTIONS "SPDLOG_FMT_EXTERNAL ON")
CPMAddPackage(URI "gh:wolfpld/tracy@0.13.1" OPTIONS "TRACY_ENABLE ${ENABLE_PROFILING}")

# ========== Compiler Features ==========

set(SPECULA_DEFINITIONS "")
include(CheckCXXSourceCompiles)
include(CheckIncludeFiles)

check_cxx_source_compiles(
  "
#include <malloc.h>
int main() { void * ptr = _aligned_malloc(1024, 32); }
"
  HAVE__ALIGNED_MALLOC
)

check_cxx_source_compiles(
  "
#include <stdlib.h>
int main() {
  void *ptr;
  posix_memalign(&ptr, 32, 1024);
} "
  HAVE_POSIX_MEMALIGN
)

if(HAVE__ALIGNED_MALLOC)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_HAVE__ALIGNED_MALLOC")
elseif(HAVE_POSIX_MEMALIGN)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_HAVE_POSIX_MEMALIGN")
else()
  message(SEND_ERROR "Unable to find a way to allocate aligned memory")
endif()

# ========== Sources ==========

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/specula/version.hpp @ONLY)

# ========== Library ==========

add_library(specula ${headers} ${sources})
add_library(specula::specula ALIAS specula)

target_include_directories(
  specula
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(specula PUBLIC spdlog::spdlog fmt::fmt PRIVATE TracyClient cpptrace::cpptrace)

target_compile_features(specula PUBLIC cxx_std_20)

target_compile_definitions(specula PRIVATE ${SPECULA_DEFINITIONS})

set_target_properties(specula PROPERTIES OUTPUT_NAME "specula")
