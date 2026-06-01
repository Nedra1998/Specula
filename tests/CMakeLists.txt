# ========== Options ==========
# ========== Dependencies ==========

CPMAddPackage("gh:catchorg/Catch2@3.12.0")
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
include(Catch)

# ========== Sources ==========

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# ========== Executable ==========

add_executable(unit-tests ${headers} ${sources})
add_executable(specula::tests ALIAS unit-tests)

target_include_directories(unit-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(unit-tests PRIVATE specula::specula Catch2::Catch2)

target_compile_features(unit-tests PRIVATE cxx_std_20)

# ========== Testing ==========

catch_discover_tests(unit-tests)
