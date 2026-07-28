# Default variables (can be overridden via CLI, e.g., `just buildtype=release`)
buildDir := "build"
buildType := "Debug"
cmakeGenerator := "Ninja"

# By default, running just `just` will trigger the build
default: build

# --- Internal Helpers ---

# Silently checks if the build directory exists and initializes it if missing
_ensure_setup:
    @if [ ! -d "{{ buildDir }}" ]; then \
        mkdir -p "{{ buildDir }}"; \
        cmake -G "{{ cmakeGenerator }}" -S . -B "{{ buildDir }}" -DCMAKE_BUILD_TYPE={{ buildType }}; \
    fi

# --- Core Build Commands ---

# Explicitly setup the build directory. Pass extra args like `just setup -Doption=true`
setup *ARGS:
    @mkdir -p "{{ buildDir }}"
    @cmake -G "{{ cmakeGenerator }}" -S . -B "{{ buildDir }}" -DCMAKE_BUILD_TYPE={{ buildType }} {{ ARGS }}

# Reconfigure the build directory to pick up new files or option changes
reconfigure *ARGS:
    @cmake -G "{{ cmakeGenerator }}" -S . -B "{{ buildDir }}" -DCMAKE_BUILD_TYPE={{ buildType }} {{ ARGS }}

# Compile the project
build: _ensure_setup
    @cmake --build "{{ buildDir }}"

# Clean the compiled outputs
clean: _ensure_setup
    @cmake --build "{{ buildDir }}" --target clean

# Completely destroy the build directory
scrub:
    @rm -rf "{{ buildDir }}"

# --- Testing & Code Quality ---

# Run the test suite. Pass extra args like `just test --suite my_suite`
test *ARGS: _ensure_setup
    @cmake --build "{{ buildDir }}" --target unit-tests
    @"{{ buildDir }}/tests/unit-tests" {{ ARGS }}

# Run formatter checks on the C++ and CMake codebase
format: _ensure_setup
    @cmake --build "{{ buildDir }}" --target format

# Run linters on the C++ codebase
lint: _ensure_setup
    @cmake --build "{{ buildDir }}" --target check

# --- Documentation ---

# Generate documentation (requires a 'docs' or similar custom target in meson.build)
docs: _ensure_setup
    @cmake --build "{{ buildDir }}" --target docs
