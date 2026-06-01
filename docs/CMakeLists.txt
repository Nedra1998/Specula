# ========== Options ==========
# ========== Dependencies ==========

CPMAddPackage("gh:jothepro/doxygen-awesome-css@2.4.2")

find_package(Doxygen REQUIRED dot)

# ========== Sources ==========

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

file(GLOB_RECURSE docs CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.md")

# ========== Configuration ==========

# General Configuration
set(DOXYGEN_STRIP_FROM_PATH "${PROJECT_SOURCE_DIR}")
set(DOXYGEN_STRIP_FROM_INC "${PROJECT_SOURCE_DIR}/include/specula")

set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(DOXYGEN_PROJECT_NUMBER "v${PROJECT_VERSION}")
set(DOXYGEN_PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.svg")
set(DOXYGEN_FILE_VERSION_FILTER "git log -n 1 --pretty=format:%h --")

set(DOXYGEN_USE_MATHJAX YES)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_DISABLE_INDEX NO)
set(DOXYGEN_FULL_SIDEBAR NO)

set(DOXYGEN_CALL_GRAPH YES)
set(DOXYGEN_SHOW_ENUM_VALUES YES)
set(DOXYGEN_HTML_COPY_CLIPBOARD NO)

set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
set(DOXYGEN_PREDEFINED "SPECULA_CPU_GPU=")

# GraphViz Configuration
set(DOXYGEN_HAVE_DOT YES)
set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
set(DOXYGEN_DOT_TRANSPARENT YES)
set(DOXYGEN_DOT_FONTNAME "Iosevka")

# # HTML Configuration
set(DOXYGEN_HTML_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/assets/header.html")
set(DOXYGEN_HTML_COLORSTYLE "LIGHT")

set(
  DOXYGEN_HTML_EXTRA_FILES
  "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js"
  "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js"
)

set(
  DOXYGEN_HTML_EXTRA_STYLESHEET
  "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css"
  "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only.css"
  "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only-darkmode-toggle.css"
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/override.css"
)

# ========== Documentation ==========

doxygen_add_docs(docs ${headers} ${sources} ${docs})
