# ========== Options ==========
# ========== Dependencies ==========

CPMAddPackage("gh:jarro2783/cxxopts@3.3.1")
CPMAddPackage("gh:Neargye/magic_enum@0.9.8")
CPMAddPackage("gh:lvandeve/lodepng#master")

# ========== Executable ==========
add_library(tools INTERFACE)
target_link_libraries(tools INTERFACE spdlog::spdlog fmt::fmt cxxopts::cxxopts magic_enum::magic_enum)
target_include_directories(tools INTERFACE "${lodepng_SOURCE_DIR}")
target_compile_features(tools INTERFACE cxx_std_20)

add_executable(tool-bluenoise bluenoise.cpp)
target_link_libraries(tool-bluenoise PUBLIC tools)

add_executable(tool-rgb2spec rgb2spec.cpp)
target_link_libraries(tool-rgb2spec PUBLIC tools)
