# cmake/FindAllDependencies.cmake
# Findet alle Abhängigkeiten mit System/vcpkg Fallback

include(FindPackageHandleStandardArgs)

# Helper-Funktion um vcpkg-Verfügbarkeit zu prüfen
function(is_vcpkg_available RESULT_VAR)
    if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# NCURSES mit vcpkg Fallback
# ============================================================================
function(find_ncurses_dependency)
    message(STATUS "Suche ncurses...")
    
    # Erst System pkg-config versuchen
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(NCURSES_SYS ncursesw)
        if(NOT NCURSES_SYS_FOUND)
            pkg_check_modules(NCURSES_SYS ncurses)
        endif()
        
        if(NCURSES_SYS_FOUND)
            set(NCURSES_LIBS ${NCURSES_SYS_LIBRARIES} PARENT_SCOPE)
            set(NCURSES_INCLUDE_DIRS ${NCURSES_SYS_INCLUDE_DIRS} PARENT_SCOPE)
            set(NCURSES_COMPILE_FLAGS ${NCURSES_SYS_CFLAGS_OTHER} PARENT_SCOPE)
            set(NCURSES_FOUND TRUE PARENT_SCOPE)
            set(NCURSES_SOURCE "system" PARENT_SCOPE)
            
            # Unicode-Support prüfen
            if(NCURSES_SYS_LIBRARIES MATCHES "ncursesw")
                set(NCURSES_UNICODE_SUPPORT TRUE PARENT_SCOPE)
            endif()
            return()
        endif()
    endif()
    
    # Fallback zu vcpkg
    find_package(Curses QUIET)
    if(CURSES_FOUND)
        set(NCURSES_LIBS ${CURSES_LIBRARIES} PARENT_SCOPE)
        set(NCURSES_INCLUDE_DIRS ${CURSES_INCLUDE_DIRS} PARENT_SCOPE)
        set(NCURSES_COMPILE_FLAGS ${CURSES_CFLAGS} PARENT_SCOPE)
        set(NCURSES_FOUND TRUE PARENT_SCOPE)
        set(NCURSES_SOURCE "vcpkg" PARENT_SCOPE)
        set(NCURSES_UNICODE_SUPPORT TRUE PARENT_SCOPE)  # vcpkg hat meist Unicode-Support
        return()
    endif()
    
    # Nicht gefunden
    set(NCURSES_FOUND FALSE PARENT_SCOPE)
    set(NCURSES_ERROR "ncurses nicht gefunden! Installiere mit: sudo apt install libncursesw5-dev ODER vcpkg install ncurses" PARENT_SCOPE)
endfunction()

# ============================================================================
# FTXUI mit vcpkg Fallback
# ============================================================================
function(find_ftxui_dependency)
    message(STATUS "Suche FTXUI...")
    
    # vcpkg zuerst versuchen (FTXUI ist neuer, vcpkg hat meist bessere Version)
    is_vcpkg_available(VCPKG_AVAILABLE)
    if(VCPKG_AVAILABLE)
        find_package(ftxui CONFIG QUIET)
        if(ftxui_FOUND)
            set(FTXUI_LIBS ftxui::dom ftxui::screen ftxui::component PARENT_SCOPE)
            set(FTXUI_INCLUDE_DIRS "" PARENT_SCOPE)
            set(FTXUI_COMPILE_FLAGS "" PARENT_SCOPE)
            set(FTXUI_FOUND TRUE PARENT_SCOPE)
            set(FTXUI_SOURCE "vcpkg" PARENT_SCOPE)
            return()
        endif()
    endif()
    
    # Fallback zu System pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FTXUI_SYS ftxui)
        if(FTXUI_SYS_FOUND)
            set(FTXUI_LIBS ${FTXUI_SYS_LIBRARIES} PARENT_SCOPE)
            set(FTXUI_INCLUDE_DIRS ${FTXUI_SYS_INCLUDE_DIRS} PARENT_SCOPE)
            set(FTXUI_COMPILE_FLAGS ${FTXUI_SYS_CFLAGS_OTHER} PARENT_SCOPE)
            set(FTXUI_FOUND TRUE PARENT_SCOPE)
            set(FTXUI_SOURCE "system" PARENT_SCOPE)
            return()
        endif()
    endif()
    
    # Nicht gefunden
    set(FTXUI_FOUND FALSE PARENT_SCOPE)
    set(FTXUI_ERROR "FTXUI nicht gefunden! Installiere mit: sudo apt install libftxui-dev ODER vcpkg install ftxui" PARENT_SCOPE)
endfunction()

# ============================================================================
# CMARK-GFM mit vcpkg Fallback
# ============================================================================
function(find_cmark_gfm_dependency)
    message(STATUS "Suche cmark-gfm...")
    
    # Erst System pkg-config versuchen
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(CMARK_GFM_SYS libcmark-gfm)
        pkg_check_modules(CMARK_GFM_EXT_SYS libcmark-gfm-extensions)
        
        if(CMARK_GFM_SYS_FOUND AND CMARK_GFM_EXT_SYS_FOUND)
            set(CMARK_GFM_LIBS ${CMARK_GFM_SYS_LIBRARIES} ${CMARK_GFM_EXT_SYS_LIBRARIES} PARENT_SCOPE)
            set(CMARK_GFM_INCLUDE_DIRS ${CMARK_GFM_SYS_INCLUDE_DIRS} ${CMARK_GFM_EXT_SYS_INCLUDE_DIRS} PARENT_SCOPE)
            set(CMARK_GFM_COMPILE_FLAGS ${CMARK_GFM_SYS_CFLAGS_OTHER} ${CMARK_GFM_EXT_SYS_CFLAGS_OTHER} PARENT_SCOPE)
            set(CMARK_GFM_FOUND TRUE PARENT_SCOPE)
            set(CMARK_GFM_SOURCE "system" PARENT_SCOPE)
            
            # Duplikate entfernen
            list(REMOVE_DUPLICATES CMARK_GFM_INCLUDE_DIRS)
            set(CMARK_GFM_INCLUDE_DIRS ${CMARK_GFM_INCLUDE_DIRS} PARENT_SCOPE)
            return()
        endif()
    endif()
    
    # Fallback zu vcpkg
    find_package(cmark-gfm CONFIG QUIET)
    find_package(cmark-gfm-extensions CONFIG QUIET)
    
    if(cmark-gfm_FOUND AND cmark-gfm-extensions_FOUND)
        set(CMARK_GFM_LIBS libcmark-gfm_static libcmark-gfm-extensions_static PARENT_SCOPE)
        set(CMARK_GFM_INCLUDE_DIRS "" PARENT_SCOPE)
        set(CMARK_GFM_COMPILE_FLAGS "" PARENT_SCOPE)
        set(CMARK_GFM_FOUND TRUE PARENT_SCOPE)
        set(CMARK_GFM_SOURCE "vcpkg" PARENT_SCOPE)
        return()
    endif()
    
    # Nicht gefunden
    set(CMARK_GFM_FOUND FALSE PARENT_SCOPE)
    set(CMARK_GFM_ERROR "cmark-gfm nicht gefunden! Installiere mit: sudo apt install libcmark-gfm-dev ODER vcpkg install cmark-gfm" PARENT_SCOPE)
endfunction()

# ============================================================================
# Haupt-Funktion: Alle Abhängigkeiten finden
# ============================================================================
function(find_all_dependencies)
    message(STATUS "=== Suche Abhängigkeiten ===")

    # ncurses finden
    find_ncurses_dependency()
    if(NCURSES_FOUND)
        message(STATUS "✓ ncurses gefunden: ${NCURSES_SOURCE}")
        if(NCURSES_UNICODE_SUPPORT)
            message(STATUS "  → Unicode-Support: aktiviert")
        endif()
    else()
        message(FATAL_ERROR "✗ ${NCURSES_ERROR}")
    endif()

    # FTXUI finden (nur wenn aktiviert)
    if(USE_FTXUI)
        find_ftxui_dependency()
        if(FTXUI_FOUND)
            message(STATUS "✓ FTXUI gefunden: ${FTXUI_SOURCE}")
            set(RENDERER_LIBS ${FTXUI_LIBS} PARENT_SCOPE)
            set(RENDERER_INCLUDE_DIRS ${FTXUI_INCLUDE_DIRS} PARENT_SCOPE)
            set(RENDERER_COMPILE_FLAGS ${FTXUI_COMPILE_FLAGS} PARENT_SCOPE)
            set(RENDERER_SOURCES src/ftxui_renderer.cc PARENT_SCOPE)
            add_compile_definitions(USE_FTXUI_RENDERER)
        else()
            message(FATAL_ERROR "✗ ${FTXUI_ERROR}")
        endif()
    else()
        message(STATUS "✓ ncurses als Renderer gewählt")
        set(RENDERER_LIBS ${NCURSES_LIBS} PARENT_SCOPE)
        set(RENDERER_INCLUDE_DIRS ${NCURSES_INCLUDE_DIRS} PARENT_SCOPE)
        set(RENDERER_COMPILE_FLAGS ${NCURSES_COMPILE_FLAGS} PARENT_SCOPE)
        set(RENDERER_SOURCES src/ncurses_renderer.cc PARENT_SCOPE)
        
        if(NCURSES_UNICODE_SUPPORT)
            add_compile_definitions(UNICODE_SUPPORT=1)
        endif()
    endif()

    # cmark-gfm finden
    find_cmark_gfm_dependency()
    if(CMARK_GFM_FOUND)
        message(STATUS "✓ cmark-gfm gefunden: ${CMARK_GFM_SOURCE}")
        set(CMARK_GFM_LIBS ${CMARK_GFM_LIBS} PARENT_SCOPE)
        set(CMARK_GFM_INCLUDE_DIRS ${CMARK_GFM_INCLUDE_DIRS} PARENT_SCOPE)
        set(CMARK_GFM_COMPILE_FLAGS ${CMARK_GFM_COMPILE_FLAGS} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "✗ ${CMARK_GFM_ERROR}")
    endif()

    message(STATUS "=== Alle Abhängigkeiten gefunden ===")
endfunction()

# Haupt-Funktion aufrufen
find_all_dependencies()