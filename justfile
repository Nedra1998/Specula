# Default variables (can be overridden via CLI, e.g., `just buildtype=release`)
buildDir := "build"
buildType := "Debug"
cmakeGenerator := "Ninja"

# By default, running just `just` will trigger the build
[private]
default: build

# Show the available recipes
[group('help')]
help:
    @just --list --unsorted

# --- Internal Helpers ---

# Silently checks if the build directory exists and initializes it if missing
_ensure_setup:
    @if [ ! -d "{{ buildDir }}" ]; then \
        mkdir -p "{{ buildDir }}"; \
        cmake -G "{{ cmakeGenerator }}" -S . -B "{{ buildDir }}" -DCMAKE_BUILD_TYPE={{ buildType }}; \
    fi

# --- Core Build Commands ---

# Explicitly setup the build directory. Pass extra args like `just setup -Doption=true`
[group('build')]
setup *ARGS:
    @mkdir -p "{{ buildDir }}"
    @cmake -G "{{ cmakeGenerator }}" -S . -B "{{ buildDir }}" -DCMAKE_BUILD_TYPE={{ buildType }} {{ ARGS }}

# Reconfigure the build directory to pick up new files or option changes
[group('build')]
reconfigure *ARGS:
    @cmake -G "{{ cmakeGenerator }}" -S . -B "{{ buildDir }}" -DCMAKE_BUILD_TYPE={{ buildType }} {{ ARGS }}

# Compile the project
[group('build')]
build: _ensure_setup
    @cmake --build "{{ buildDir }}"

# Clean the compiled outputs
[group('build')]
clean: _ensure_setup
    @cmake --build "{{ buildDir }}" --target clean

# Completely destroy the build directory
[group('build')]
scrub:
    @rm -rf "{{ buildDir }}"

# --- Testing & Code Quality ---

# Run the test suite
[group('test')]
test *ARGS: _ensure_setup
    @cmake --build "{{ buildDir }}" --target unit-tests
    @{{ buildDir }}/tests/unit-tests {{ ARGS }}

# Run the test suite under gdb
[group('test')]
gdb *ARGS: _ensure_setup
    @cmake --build "{{ buildDir }}" --target unit-tests
    @gdb --args {{ buildDir }}/tests/unit-tests --break {{ ARGS }}

# Run formatter checks on the C++ and CMake codebase
[group('quality')]
format: _ensure_setup
    @cmake --build "{{ buildDir }}" --target format

# Run linters on the C++ codebase
[group('quality')]
lint: _ensure_setup
    @cmake --build "{{ buildDir }}" --target check

# --- Documentation ---

# Generate documentation (requires configuring with -DSPECULA_BUILD_DOCS=ON)
[group('docs')]
docs: _ensure_setup
    @cmake --build "{{ buildDir }}" --target docs

# Regenerate documentation whenever a file in include/, src/ or docs/ changes (requires entr)
[group('docs')]
docs-live: docs
    @# entr exits 0 on quit, 2 after rebuilding for a new file, and 1 without rebuilding for a deleted one
    @while find include src docs -type f | entr -dp cmake --build "{{ buildDir }}" --target docs; status=$?; [ $status -ne 0 ]; do \
        [ $status -eq 2 ] || cmake --build "{{ buildDir }}" --target docs; \
    done
