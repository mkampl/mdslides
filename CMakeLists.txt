cmake_minimum_required(VERSION 3.15)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    # Check environment variable first
    if(DEFINED ENV{VCPKG_ROOT})
        set(VCPKG_TOOLCHAIN "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        if(EXISTS ${VCPKG_TOOLCHAIN})
            set(CMAKE_TOOLCHAIN_FILE ${VCPKG_TOOLCHAIN})
            message(STATUS "Using vcpkg from VCPKG_ROOT: ${CMAKE_TOOLCHAIN_FILE}")
        endif()
    else()
        # Try common locations
        set(VCPKG_PATHS
            "$ENV{HOME}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake"
            "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake"
            "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake"
            "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake"
        )
        
        foreach(VCPKG_PATH ${VCPKG_PATHS})
            if(EXISTS ${VCPKG_PATH})
                set(CMAKE_TOOLCHAIN_FILE ${VCPKG_PATH})
                message(STATUS "Auto-detected vcpkg: ${VCPKG_PATH}")
                break()
            endif()
        endforeach()
    endif()
endif()

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(STATUS "vcpkg not found - using system packages only")
else()
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(MarkdownSlidePresenter VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(USE_FTXUI "Use FTXUI instead of ncurses" OFF)

# Find packages using system pkg-config
find_package(PkgConfig REQUIRED)

pkg_check_modules(CMARK_GFM libcmark-gfm)
pkg_check_modules(CMARK_GFM_EXT libcmark-gfm-extensions)

if(CMARK_GFM_FOUND AND CMARK_GFM_EXT_FOUND)
    # Use system packages
    set(CMARK_LIBS ${CMARK_GFM_LIBRARIES} ${CMARK_GFM_EXT_LIBRARIES})
    set(CMARK_INCLUDE_DIRS ${CMARK_GFM_INCLUDE_DIRS} ${CMARK_GFM_EXT_INCLUDE_DIRS})
    set(CMARK_COMPILE_FLAGS ${CMARK_GFM_CFLAGS_OTHER} ${CMARK_GFM_EXT_CFLAGS_OTHER})
    message(STATUS "Found cmark-gfm: system")
else()
    # Fallback to vcpkg
    find_package(cmark-gfm CONFIG QUIET)
    find_package(cmark-gfm-extensions CONFIG QUIET)
    
    if(cmark-gfm_FOUND AND cmark-gfm-extensions_FOUND)
        set(CMARK_LIBS libcmark-gfm_static libcmark-gfm-extensions_static)
        set(CMARK_INCLUDE_DIRS "")
        set(CMARK_COMPILE_FLAGS "")
        message(STATUS "Found cmark-gfm and cmark-gfm-extensions: vcpkg fallback")
    else()
        message(FATAL_ERROR "cmark-gfm not found! Install with: sudo apt install libcmark-gfm-dev OR vcpkg install cmark-gfm")
    endif()
endif()

# Find renderer
if(USE_FTXUI)
    # Try system ftxui first
    pkg_check_modules(FTXUI ftxui)
    if(FTXUI_FOUND)
        set(RENDERER_LIBS ${FTXUI_LIBRARIES})
        set(RENDERER_SOURCES src/ftxui_renderer.cc)
        set(RENDERER_INCLUDE_DIRS ${FTXUI_INCLUDE_DIRS})
        set(RENDERER_COMPILE_FLAGS ${FTXUI_CFLAGS_OTHER})
        add_compile_definitions(USE_FTXUI_RENDERER)
        message(STATUS "Found FTXUI: system")
    else()
        message(FATAL_ERROR "FTXUI not found! Install with: sudo apt install libftxui-dev")
    endif()
else()
    # Find ncurses (prefer wide character version)
    pkg_check_modules(NCURSES ncursesw)
    if(NOT NCURSES_FOUND)
        pkg_check_modules(NCURSES REQUIRED ncurses)
    endif()
    
    set(RENDERER_LIBS ${NCURSES_LIBRARIES})
    set(RENDERER_SOURCES src/ncurses_renderer.cc)
    set(RENDERER_INCLUDE_DIRS ${NCURSES_INCLUDE_DIRS})
    set(RENDERER_COMPILE_FLAGS ${NCURSES_CFLAGS_OTHER})
    message(STATUS "Found ncurses: system")
    
    # Unicode support detection
    if(NCURSES_LIBRARIES MATCHES "ncursesw")
        add_compile_definitions(UNICODE_SUPPORT=1)
        message(STATUS "Unicode support: enabled")
    else()
        message(STATUS "Unicode support: disabled")
    endif()
endif()

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
    ${CMARK_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(mdslides 
    ${RENDERER_LIBS}
    ${CMARK_LIBS}
)

# Add compile flags
target_compile_options(mdslides PRIVATE 
    ${RENDERER_COMPILE_FLAGS}
    ${CMARK_COMPILE_FLAGS}
)

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