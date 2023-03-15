# ---- Dependencies ----

find_package(Doxygen REQUIRED dot)

CPMAddPackage(
  NAME doxygen-awesome-css
  GITHUB_REPOSITORY jothepro/doxygen-awesome-css
  GIT_TAG v2.2.0)

# ---- Sources ----

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/specula/*")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*")
file(GLOB_RECURSE DOCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.md")
list(APPEND DOCS "${PROJECT_SOURCE_DIR}/README.md" "${PROJECT_SOURCE_DIR}/LICENSE"
     "${PROJECT_SOURCE_DIR}/CHANGELOG.md")

# ---- Documentation ----

set(DOXYGEN_STRIP_FROM_PATH "${PROJECT_SOURCE_DIR}")
set(DOXYGEN_STRIP_FROM_INC "${PROJECT_SOURCE_DIR}/include/specula")
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")
set(DOXYGEN_PROJECT_NUMBER "v${PROJECT_VERSION}")
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_DISABLE_INDEX NO)
set(DOXYGEN_FULL_SIDEBAR NO)
set(DOXYGEN_HTML_COLORSTYLE "LIGHT")
set(DOXYGEN_PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.png")

set(DOXYGEN_GENERATE_XML YES)

set(DOXYGEN_HTML_EXTRA_STYLESHEET
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only.css"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only-darkmode-toggle.css"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/override.css")

set(DOXYGEN_HTML_EXTRA_FILES
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-interactive-toc.js"
    "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-paragraph-link.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.png")

set(DOXYGEN_LAYOUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/assets/DoxygenLayout.xml")
set(DOXYGEN_HTML_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/assets/header.html")
set(DOXYGEN_HTML_FOOTER "${CMAKE_CURRENT_SOURCE_DIR}/assets/footer.html")

set(DOXYGEN_FILE_VERSION_FILTER "git log -n 1 --pretty=format:%h --")

set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
set(DOXYGEN_DOT_IMAGE_FORMAT svg)
set(DOXYGEN_DOT_FONTNAME "Iosevka")
set(DOXYGEN_DOT_TRANSPARENT YES)
set(DOXYGEN_HTML_COLORSTYLE_HUE 209)
set(DOXYGEN_HTML_COLORSTYLE_SAT 255)
set(DOXYGEN_HTML_COLORSTYLE_GAMMA 113)

doxygen_add_docs(docs ${HEADERS} ${SOURCES} ${DOCS})

add_custom_command(
  TARGET docs
  POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/html/docs/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.png"
          "${CMAKE_CURRENT_BINARY_DIR}/html/docs/assets")
