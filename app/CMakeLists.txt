# ---- Dependencies ----

CPMAddPackage(
  NAME cxxopts
  GITHUB_REPOSITORY jarro2783/cxxopts
  GIT_TAG v3.1.1)

# ---- Sources ----

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# ---- Library ----

add_executable(app ${header} ${sources})
add_executable(specula::app ALIAS app)

set_target_properties(app PROPERTIES OUTPUT_NAME specula CXX_STANDARD 20)

target_compile_options(app PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

target_link_libraries(app PRIVATE specula::specula cxxopts::cxxopts)
target_include_directories(specula PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
