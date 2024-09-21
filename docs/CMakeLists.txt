# ===== Dependencies =====

find_package(Doxygen REQUIRED dot)

CPMAddPackage(
  NAME doxygen-awesome-css
  GITHUB_REPOSITORY jothepro/doxygen-awesome-css
  GIT_TAG v2.3.3)

# ===== Sources =====

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/specula/*")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE DOCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.md")
list(APPEND DOCS "${PROJECT_SOURCE_DIR}/LICENSE.md" "${PROJECT_SOURCE_DIR}/CHANGELOG.md")

# ===== Doxygen Configuration =====

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

# GraphViz Configuration
set(DOXYGEN_HAVE_DOT YES)
set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
set(DOXYGEN_DOT_TRANSPARENT YES)
set(DOXYGEN_DOT_FONTNAME "Iosevka")

# HTML Configuration
set(DOXYGEN_LAYOUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/assets/DoxygenLayout.xml")
set(DOXYGEN_HTML_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/assets/header.html")
set(DOXYGEN_HTML_FOOTER "${CMAKE_CURRENT_SOURCE_DIR}/assets/footer.html")
set(DOXYGEN_HTML_COLORSTYLE "LIGHT")

set(DOXYGEN_HTML_EXTRA_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/Iosevka-Regular.woff2"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-paragraph-link.js"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-interactive-toc.js")
list(APPEND DOXYGEN_HTML_EXTRA_FILES ${DOC_ASSETS})

set(DOXYGEN_HTML_EXTRA_STYLESHEET
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only.css"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only-darkmode-toggle.css"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/override.css")

# ===== Doxygen Target =====

doxygen_add_docs(docs ${HEADERS} ${SOURCES} ${DOCS})
