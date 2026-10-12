# ========== Options ==========
# ========== Dependencies ==========

CPMAddPackage("gh:fmtlib/fmt#12.1.0")
CPMAddPackage(URI "gh:jeremy-rifkin/cpptrace@1.0.4" OPTIONS "BUILD_SHARED_LIBS OFF")
CPMAddPackage("gh:nemtrif/utfcpp@4.2.0")
CPMAddPackage(URI "gh:JuliaStrings/utf8proc@2.11.3" OPTIONS "BUILD_SHARED_LIBS OFF")
CPMAddPackage(URI "gh:gabime/spdlog@1.17.0" OPTIONS "SPDLOG_FMT_EXTERNAL ON")
CPMAddPackage(URI "gh:wolfpld/tracy@0.13.1" OPTIONS "TRACY_ENABLE ${ENABLE_PROFILING}")
CPMAddPackage("gh:Neargye/magic_enum@0.9.8")
CPMAddPackage("gh:nothings/stb#master")
CPMAddPackage("gh:lvandeve/lodepng#master")
CPMAddPackage(
  URI
  "gh:academysoftwarefoundation/openexr@3.5.2"
  OPTIONS "OPENEXR_BUILD_TOOLS OFF" "OPENEXR_BUILD_EXAMPLES OFF"
)
CPMAddPackage("gh:phoboslab/qoi#master")

# ========== Compiler Features ==========

set(SPECULA_DEFINITIONS "")
include(CheckCXXSourceCompiles)
include(CheckIncludeFiles)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_IS_WINDOWS" "NOMINMAX")
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_IS_OSX")
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_IS_LINUX")
else()
  message(SEND_ERROR "Unknown system name: " + CMAKE_SYSTEM_NAME)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_DEBUG_BUILD")
endif()

check_cxx_source_compiles(
  "
#include <intrin.h>
int main() {
    unsigned long lz = 0, v = 1234;
    if (_BitScanReverse(&lz, v)) return lz;
    return 0;
} "
  HAS_INTRIN_H
)

if(HAS_INTRIN_H)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_HAS_INTRIN_H")
endif()

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

file(GLOB_RECURSE bluenoise_images CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/assets/bluenoise/*.png")
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/bluenoise.cpp"
  DEPENDS tool-bluenoise ${bluenoise_images}
  COMMAND tool-bluenoise ${bluenoise_images}
)
list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/bluenoise.cpp")

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_aces.cpp"
  DEPENDS tool-rgb2spec
  COMMAND tool-rgb2spec -g ACES2065_1 64 "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_aces.cpp"
)
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_dci_p3.cpp"
  DEPENDS tool-rgb2spec
  COMMAND tool-rgb2spec -g DCI_P3 64 "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_dci_p3.cpp"
)
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_rec2020.cpp"
  DEPENDS tool-rgb2spec
  COMMAND tool-rgb2spec -g REC2020 64 "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_rec2020.cpp"
)
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_srgb.cpp"
  DEPENDS tool-rgb2spec
  COMMAND tool-rgb2spec -g SRGB 64 "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_srgb.cpp"
)
list(
  APPEND sources
  "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_aces.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_dci_p3.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_rec2020.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/rgbspectrum_srgb.cpp"
)

# ========== Library ==========

add_library(specula ${headers} ${sources})
add_library(specula::specula ALIAS specula)

target_include_directories(
  specula
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include
  PRIVATE "${stb_SOURCE_DIR}" "${lodepng_SOURCE_DIR}" "${qoi_SOURCE_DIR}" >
)

target_link_libraries(
  specula
  PUBLIC spdlog::spdlog fmt::fmt
  PRIVATE TracyClient cpptrace::cpptrace utf8cpp utf8proc magic_enum::magic_enum OpenEXR::OpenEXR
)

target_compile_features(specula PUBLIC cxx_std_20)

target_compile_definitions(specula PRIVATE ${SPECULA_DEFINITIONS})

set_target_properties(specula PROPERTIES OUTPUT_NAME "specula")
