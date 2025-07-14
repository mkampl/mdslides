cmake_minimum_required(VERSION 3.15)
project(MarkdownSlidePresenter VERSION 1.0.0 LANGUAGES CXX)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch FTXUI
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find cmark
pkg_check_modules(CMARK REQUIRED libcmark)
if(NOT CMARK_FOUND)
    message(FATAL_ERROR "libcmark not found! Please install cmark development package.")
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/markdown_parser.cpp
    src/slide_renderer.cpp
    src/theme_manager.cpp
    src/shell_executor.cpp
    src/presentation_controller.cpp
)

set(HEADERS
    include/markdown_parser.hpp
    include/slide_renderer.hpp
    include/theme_manager.hpp
    include/shell_executor.hpp
    include/presentation_controller.hpp
    include/slide_types.hpp
)

# Create executable
add_executable(mdslides ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(mdslides PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMARK_INCLUDE_DIRS}
)

# Link libraries (FTXUI component must be first in linking order)
target_link_libraries(mdslides
    PRIVATE ftxui::component
    PRIVATE ftxui::dom
    PRIVATE ftxui::screen
    ${CMARK_LIBRARIES}
)

# Add compile definitions
target_compile_definitions(mdslides PRIVATE
    ${CMARK_CFLAGS_OTHER}
)

# Installation
install(TARGETS mdslides RUNTIME DESTINATION bin)
install(FILES README.md DESTINATION share/doc/mdslides)

# Create directories for headers during build
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Package configuration
set(CPACK_PACKAGE_NAME "MarkdownSlidePresenter")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern terminal-based markdown slide presenter")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcmark0 | libcmark-dev")
endif()

include(CPack)
