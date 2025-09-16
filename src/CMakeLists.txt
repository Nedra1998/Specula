# ===== Dependencies =====

CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG 11.2.0 EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

CPMAddPackage(
  NAME magic_enum
  GITHUB_REPOSITORY Neargye/magic_enum
  GIT_TAG v0.9.7 EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  GIT_TAG v1.15.3
  OPTIONS "SPDLOG_FMT_EXTERNAL ON" EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

CPMAddPackage(
  NAME tracy
  GITHUB_REPOSITORY wolfpld/tracy
  GIT_TAG v0.12.2
  OPTIONS "TRACY_ENABLE ${ENABLE_PROFILING}" EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

CPMAddPackage(
  NAME cpptrace
  GITHUB_REPOSITORY jeremy-rifkin/cpptrace
  GIT_TAG v1.0.4 EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

# ===== Compiler Features =====

set(SPECULA_DEFINITIONS "")
include(CheckCXXSourceCompiles)
include(CheckIncludeFiles)

check_cxx_source_compiles(
  "
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
int main() {
   int fd = open(\"foo\", O_RDONLY);
   struct stat s;
   fstat(fd, &s);
   size_t len = s.st_size;
   void *ptr = mmap(0, len, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
   munmap(ptr, len);
}
"
  HAVE_MMAP)

if(HAVE_MMAP)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_HAVE_MMAP")
endif()

include(CheckIncludeFiles)

check_cxx_source_compiles(
  "
#include <intrin.h>
int main() {
    unsigned long lz = 0, v = 1234;
    if (_BitScanReverse(&lz, v)) return lz;
    return 0;
} "
  HAS_INTRIN_H)

if(HAS_INTRIN_H)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_HAS_INTRIN_H")
endif()

check_cxx_source_compiles("__declspec(noinline) void foo() { }
int main() { }" HAVE_DECLSPEC_NOINLINE)

check_cxx_source_compiles("__attribute__((noinline)) void foo() { }
int main() { }" HAVE_ATTRIBUTE_NOINLINE)

if(HAVE_ATTRIBUTE_NOINLINE)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_NOINLINE=__attribute__((noinline))")
elseif(HAVE_DECLSPEC_NOINLINE)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_NOINLINE=__declspec(noinline)")
else()
  list(APPEND SPECULA_DEFINITIONS "SPECULA_NOINLINE")
endif()

check_cxx_source_compiles("int * __restrict__ ptr;
int main() { }" HAVE_PRE_POST_BAR_RESTRICT)

check_cxx_source_compiles("int * __restrict ptr;
int main() { }" HAVE_PRE_BAR_RESTRICT)

if(HAVE_PRE_POST_BAR_RESTRICT)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_RESTRICT=__restrict__")
elseif(HAVE_PRE_BAR_RESTRICT)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_RESTRICT=__restrict")
else()
  list(APPEND SPECULA_DEFINITIONS "SPECULA_RESTRICT")
endif()

check_cxx_source_compiles(
  "
#include <malloc.h>
int main() { void * ptr = _aligned_malloc(1024, 32); }
"
  HAVE__ALIGNED_MALLOC)

check_cxx_source_compiles(
  "
#include <stdlib.h>
int main() {
  void *ptr;
  posix_memalign(&ptr, 32, 1024);
} "
  HAVE_POSIX_MEMALIGN)

if(HAVE__ALIGNED_MALLOC)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_HAVE__ALIGNED_MALLOC")
elseif(HAVE_POSIX_MEMALIGN)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_HAVE_POSIX_MEMALIGN")
else()
  message(SEND_ERROR "Unable to find a way to allocate aligned memory")
endif()

check_cxx_source_compiles(
  "
#include <cstdint>
#include <type_traits>
static_assert(!std::is_same<long, int64_t>::value && !std::is_same<long long, int64_t>::value);
int main() { }
"
  INT64_IS_OWN_TYPE)

if(INT64_IS_OWN_TYPE)
  list(APPEND SPECULA_DEFINITIONS "SPECULA_INT64_IS_OWN_TYPE")
endif()

# ===== Sources =====

include(GenerateExportHeader)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

string(TOUPPER ${PROJECT_NAME} UPPERCASE_PROJECT_NAME)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/specula/version.hpp)

# ===== Library =====

add_library(specula ${header} ${sources})
add_library(specula::specula ALIAS specula)

generate_export_header(specula EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/include/specula/specula_export.hpp)

set_target_properties(specula PROPERTIES OUTPUT_NAME specula)
target_compile_features(specula PUBLIC cxx_std_20)

target_compile_definitions(
  specula
  PUBLIC -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
  PRIVATE -DSPECULA_STATIC_DEFINE ${SPECULA_DEFINITIONS})

target_link_libraries(
  specula
  PUBLIC sanitizers spdlog::spdlog fmt::fmt TracyClient magic_enum::magic_enum
  PRIVATE cpptrace::cpptrace)

target_include_directories(
  specula
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>
  PRIVATE ${PROJECT_SOURCE_DIR}/include/specula)

if(WIN32)
  add_custom_command(
    TARGET specula
    COMMENT "Copying cpptrace DLL"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:cpptrace::cpptrace>
            $<TARGET_FILE_DIR:specula>)
endif()

if(ENABLE_COVERAGE)
  enable_coverage(specula)
endif()

# ===== Linting =====

if(ENABLE_IWYU)
  set_target_properties(specula PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${IWYU_EXECUTABLE}")
endif()

if(ENABLE_LWYU)
  set_target_properties(specula PROPERTIES CXX_LINK_WHAT_YOU_USE "TRUE")
endif()

if(ENABLE_CLANG_TIDY)
  set_target_properties(specula PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}")
endif()

if(ENABLE_CPPCHECK)
  set_target_properties(specula PROPERTIES CXX_CPPCHECK "${CPPCHECK_EXECUTABLE}")
endif()
