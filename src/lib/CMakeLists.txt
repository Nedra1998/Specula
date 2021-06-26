file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

include(GenerateVersionHeader)
generate_version_header(OUTPUT_DIR
                        "${CMAKE_CURRENT_SOURCE_DIR}/include/specula/")
list(APPEND INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include/common/version.hpp")

add_library(specula ${SOURCES})

target_compile_features(specula PUBLIC cxx_std_20)
target_link_libraries(specula PUBLIC fmt spdlog)

# target_precompile_headers(specula PRIVATE <string> <fmt/format.h>)

target_include_directories(specula PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(
  specula PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/specula")

add_sycl_to_target(TARGET specula)
enable_extra_compiler_warnings(specula)

if(CODE_COVERAGE)
  enable_code_coverage(specula)
endif()

if(ENABLE_FORMATTING)
  format_target(specula TEST)
endif()
if(ENABLE_ANALYSIS)
  analyze_target(specula TEST)
endif()
