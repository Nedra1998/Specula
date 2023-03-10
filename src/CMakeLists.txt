# ---- Dependencies ----

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  GIT_TAG b25aaecf6aae27a3ccae860fadb952b3cfb40dd1)

# ---- Sources ----

include(GenerateExportHeader)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

string(TOUPPER ${PROJECT_NAME} UPPERCASE_PROJECT_NAME)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/specula/version.hpp)

# ---- Library ----

add_library(specula ${header} ${sources})
add_library(specula::specula ALIAS specula)

generate_export_header(specula EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/include/specula/specula_export.hpp)

set_target_properties(specula PROPERTIES OUTPUT_NAME specula)
target_compile_features(specula PUBLIC cxx_std_20)

target_compile_options(specula PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

target_compile_definitions(specula PRIVATE -DSPECULA_STATIC_DEFINE)

target_link_libraries(specula PUBLIC spdlog::spdlog)

target_include_directories(
  specula
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>)

# ---- Analysis & Linting ----
