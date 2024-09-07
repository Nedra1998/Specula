# ===== Dependencies =====

CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG 11.0.2 EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  GIT_TAG v1.14.1
  OPTIONS "SPDLOG_FMT_EXTERNAL ON" EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

CPMAddPackage(
  NAME tracy
  GITHUB_REPOSITORY wolfpld/tracy
  GIT_TAG v0.11.0
  OPTIONS "TRACY_ENABLE ${ENABLE_PROFILING}" EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

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
  PRIVATE -DSPECULA_STATIC_DEFINE)

target_link_libraries(specula PUBLIC sanitizers spdlog::spdlog fmt::fmt TracyClient)

target_include_directories(
  specula
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>
  PRIVATE ${PROJECT_SOURCE_DIR}/include/specula)

# ===== Linting =====

if(ENABLE_IWYU)
  set_target_properties(specula PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${IWYU_FLAGS}")
endif()

if(ENABLE_LWYU)
  set_target_properties(specula PROPERTIES CXX_LINK_WHAT_YOU_USE "${LWYU_FLAGS}")
endif()

if(ENABLE_CLANG_TIDY)
  set_target_properties(specula PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_FLAGS}")
endif()

if(ENABLE_CPPCHECK)
  set_target_properties(specula PROPERTIES CXX_CPPCHECK "${CPPCHECK_FLAGS}")
endif()
