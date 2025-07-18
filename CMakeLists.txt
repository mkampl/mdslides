cmake_minimum_required(VERSION 3.15)

# Load vcpkg configuration
include(cmake/vcpkg.cmake OPTIONAL)

project(MarkdownSlidePresenter VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(USE_FTXUI "Use FTXUI instead of ncurses" OFF)

# Find all dependencies
include(cmake/FindAllDependencies.cmake)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Source files
set(SOURCES
    src/main.cc
    src/markdown_parser.cc
    src/ncurses_renderer.cc
    src/shell_command_selector.cc
    src/shell_popup.cc
    src/slide_element.cc
    src/slide_renderer.cc
    src/theme_config.cc
    ${RENDERER_SOURCES}
)

# Create executable
add_executable(mdslides ${SOURCES})

# Include directories
target_include_directories(mdslides PRIVATE 
    include
    ${RENDERER_INCLUDE_DIRS}
    ${CMARK_GFM_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(mdslides 
    ${RENDERER_LIBS}
    ${CMARK_GFM_LIBS}
)

# Add compile flags
if(RENDERER_COMPILE_FLAGS)
    target_compile_options(mdslides PRIVATE ${RENDERER_COMPILE_FLAGS})
endif()
if(CMARK_GFM_COMPILE_FLAGS)
    target_compile_options(mdslides PRIVATE ${CMARK_GFM_COMPILE_FLAGS})
endif()

# Installation
install(TARGETS mdslides RUNTIME DESTINATION bin)
install(FILES README.md LICENSE LICENSE_3rdPARTY 
        DESTINATION share/doc/mdslides)

# Package info
set(CPACK_PACKAGE_NAME "MarkdownSlidePresenter")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Terminal-based markdown slide presenter")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Markus Kampl <markus.kampl@gmail.com>")

# Debian package dependencies
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libncurses6, libcmark-gfm-dev")

include(CPack)