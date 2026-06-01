# ========== Options ==========
# ========== Dependencies ==========

CPMAddPackage("gh:jarro2783/cxxopts@3.3.1")

# ========== Sources ==========

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# ========== Executable ==========

add_executable(app ${headers} ${sources})
add_executable(specula::app ALIAS app)

target_include_directories(app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(app PRIVATE specula::specula cxxopts::cxxopts)

target_compile_features(app PUBLIC cxx_std_20)

set_target_properties(app PROPERTIES OUTPUT_NAME "specula")
