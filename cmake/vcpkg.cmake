# cmake/vcpkg.cmake - Einfache vcpkg Auto-Erkennung

# Nur vcpkg-Toolchain automatisch setzen, wenn nicht bereits gesetzt
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    # Versuche gängige vcpkg-Standorte
    set(VCPKG_PATHS
        "$ENV{HOME}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake"
        "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake"
        "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake"
        "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake"
    )
    
    # Prüfe Umgebungsvariable VCPKG_ROOT zuerst
    if(DEFINED ENV{VCPKG_ROOT})
        set(VCPKG_ENV_PATH "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        if(EXISTS ${VCPKG_ENV_PATH})
            set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ENV_PATH})
            message(STATUS "vcpkg gefunden via VCPKG_ROOT: ${CMAKE_TOOLCHAIN_FILE}")
            return()
        endif()
    endif()
    
    # Durchsuche Standard-Pfade
    foreach(VCPKG_PATH ${VCPKG_PATHS})
        if(EXISTS ${VCPKG_PATH})
            set(CMAKE_TOOLCHAIN_FILE ${VCPKG_PATH})
            message(STATUS "vcpkg auto-erkannt: ${VCPKG_PATH}")
            return()
        endif()
    endforeach()
    
    message(STATUS "vcpkg nicht gefunden - verwende nur System-Pakete")
endif()