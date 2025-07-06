# ===== Dependencies =====

CPMAddPackage(
  NAME cxxopts
  GITHUB_REPOSITORY jarro2783/cxxopts
  GIT_TAG v3.3.1)

# ===== Sources =====

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# ===== Executable =====

add_executable(app ${header} ${sources})
add_executable(specula::app ALIAS app)

set_target_properties(app PROPERTIES OUTPUT_NAME specula)

target_link_libraries(app PRIVATE specula::specula cxxopts::cxxopts)
target_include_directories(specula PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
