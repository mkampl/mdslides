cmake_minimum_required(VERSION 3.15)
project(MarkdownSlidePresenter VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find ncurses library
if(WIN32)
    # For Windows, we'll need to handle ncurses differently
    # PDCurses is a common alternative for Windows
    find_library(CURSES_LIBRARY
        NAMES pdcurses curses ncurses
        PATHS 
            ${CMAKE_PREFIX_PATH}/lib
            /usr/local/lib
            /usr/lib
    )
    find_path(CURSES_INCLUDE_DIR
        NAMES curses.h ncurses.h
        PATHS 
            ${CMAKE_PREFIX_PATH}/include
            /usr/local/include
            /usr/include
    )
    
    if(CURSES_LIBRARY AND CURSES_INCLUDE_DIR)
        set(CURSES_FOUND TRUE)
    endif()
else()
    # Unix-like systems (Linux, macOS, BSD)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        # Try ncursesw first (wide character support)
        pkg_check_modules(NCURSES ncursesw)
        if(NOT NCURSES_FOUND)
            pkg_check_modules(NCURSES ncurses)
        endif()
    endif()
    
    # Fallback to manual search if pkg-config fails
    if(NOT NCURSES_FOUND)
        find_library(CURSES_LIBRARY
            NAMES ncursesw ncurses curses
            PATHS /usr/lib /usr/local/lib /opt/local/lib
        )
        find_path(CURSES_INCLUDE_DIR
            NAMES ncurses.h curses.h
            PATHS /usr/include /usr/local/include /opt/local/include
            PATH_SUFFIXES ncurses ncursesw
        )
        
        if(CURSES_LIBRARY AND CURSES_INCLUDE_DIR)
            set(CURSES_FOUND TRUE)
        endif()
    endif()
endif()

# Check if we found ncurses
if(NCURSES_FOUND)
    message(STATUS "Found ncurses via pkg-config")
    set(CURSES_LIBRARIES ${NCURSES_LIBRARIES})
    set(CURSES_INCLUDE_DIRS ${NCURSES_INCLUDE_DIRS})
    set(CURSES_COMPILE_OPTIONS ${NCURSES_CFLAGS_OTHER})
elseif(CURSES_FOUND)
    message(STATUS "Found curses library: ${CURSES_LIBRARY}")
    set(CURSES_LIBRARIES ${CURSES_LIBRARY})
    set(CURSES_INCLUDE_DIRS ${CURSES_INCLUDE_DIR})
    set(CURSES_COMPILE_OPTIONS "")
else()
    message(FATAL_ERROR "ncurses library not found! Please install ncurses development packages.")
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
endif()

# Executable
add_executable(slides slides.cpp)

# Link libraries and include directories
target_link_libraries(slides ${CURSES_LIBRARIES})
if(CURSES_INCLUDE_DIRS)
    target_include_directories(slides PRIVATE ${CURSES_INCLUDE_DIRS})
endif()
if(CURSES_COMPILE_OPTIONS)
    target_compile_options(slides PRIVATE ${CURSES_COMPILE_OPTIONS})
endif()

# Platform-specific linking
if(WIN32)
    # Windows might need additional libraries
    target_link_libraries(slides ws2_32)
elseif(APPLE)
    # macOS specific settings
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    if(COREFOUNDATION_LIBRARY)
        target_link_libraries(slides ${COREFOUNDATION_LIBRARY})
    endif()
endif()

# Installation
install(TARGETS slides
        RUNTIME DESTINATION bin)

# Install example config file
install(FILES slide_config.txt
        DESTINATION share/slides
        OPTIONAL)

# Install documentation
install(FILES README.md
        DESTINATION share/doc/slides)

# Package configuration
set(CPACK_PACKAGE_NAME "MarkdownSlidePresenter")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Terminal-based markdown slide presenter")
set(CPACK_PACKAGE_VENDOR "Open Source")
set(CPACK_PACKAGE_CONTACT "developer@example.com")

# Platform-specific package generators
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Markdown Slide Presenter")
    set(CPACK_NSIS_HELP_LINK "https://github.com/yourrepo")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/yourrepo")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "Markdown Slide Presenter")
else()
    # Linux and other Unix-like systems
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libncurses6 | libncurses5")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Text")
    set(CPACK_RPM_PACKAGE_REQUIRES "ncurses")
endif()

include(CPack)
