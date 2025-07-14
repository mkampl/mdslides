cmake_minimum_required(VERSION 3.15)
project(MarkdownSlidePresenter VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find ncurses library with Unicode support preference
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    # Try to find ncursesw (wide character support) first, then fallback to ncurses
    pkg_check_modules(NCURSES ncursesw)
    if(NOT NCURSES_FOUND)
        pkg_check_modules(NCURSES ncurses)
    endif()
endif()

if(NOT NCURSES_FOUND)
    # Manual search with preference for wide character version
    find_library(CURSES_LIBRARY
        NAMES ncursesw ncurses curses pdcurses
        PATHS /usr/lib /usr/local/lib /opt/local/lib
    )
    find_path(CURSES_INCLUDE_DIR
        NAMES ncurses.h curses.h
        PATHS /usr/include /usr/local/include /opt/local/include
        PATH_SUFFIXES ncurses ncursesw
    )
    
    if(CURSES_LIBRARY AND CURSES_INCLUDE_DIR)
        set(CURSES_FOUND TRUE)
        set(CURSES_LIBRARIES ${CURSES_LIBRARY})
        set(CURSES_INCLUDE_DIRS ${CURSES_INCLUDE_DIR})
    endif()
else()
    set(CURSES_FOUND TRUE)
    set(CURSES_LIBRARIES ${NCURSES_LIBRARIES})
    set(CURSES_INCLUDE_DIRS ${NCURSES_INCLUDE_DIRS})
endif()

if(NOT CURSES_FOUND)
    message(FATAL_ERROR "ncurses library not found! Please install ncurses development packages.")
endif()

# Determine if we found the wide character version
if(CURSES_LIBRARY MATCHES "ncursesw" OR NCURSES_LIBRARIES MATCHES "ncursesw")
    message(STATUS "Found ncursesw (Unicode support enabled)")
    set(UNICODE_SUPPORT TRUE)
else()
    message(STATUS "Found ncurses (ASCII fallback mode)")
    set(UNICODE_SUPPORT FALSE)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
endif()

# Source files
set(SOURCES
    src/main.cc
    src/slide_element.cc
    src/theme_config.cc
    src/markdown_parser.cc
    src/slide_renderer.cc
)

# Create executable
add_executable(mdslides ${SOURCES})

# Include directories
target_include_directories(mdslides PRIVATE include)

# Link libraries
target_link_libraries(mdslides ${CURSES_LIBRARIES})
if(CURSES_INCLUDE_DIRS)
    target_include_directories(mdslides PRIVATE ${CURSES_INCLUDE_DIRS})
endif()

# Add Unicode support flag if available
if(UNICODE_SUPPORT)
    target_compile_definitions(mdslides PRIVATE UNICODE_SUPPORT=1)
endif()

# Installation
install(TARGETS mdslides RUNTIME DESTINATION bin)
install(FILES README.md DESTINATION share/doc/mdslides)

# Package configuration
set(CPACK_PACKAGE_NAME "MarkdownSlidePresenter")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Terminal-based markdown slide presenter with Unicode support")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    if(UNICODE_SUPPORT)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libncursesw6 | libncursesw5")
    else()
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libncurses6 | libncurses5")
    endif()
endif()

include(CPack)
