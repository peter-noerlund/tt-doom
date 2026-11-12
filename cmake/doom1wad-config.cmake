if(NOT doom1wad_FOUND)
    set(doom1wad_FOUND OFF)

    find_file(DOOM1_WAD NAMES doom1.wad DOOM1.WAD PATHS $ENV{HOME}/.config/gzdoom)
    if(NOT DOOM1_WAD STREQUAL "DOOM1WAD-NOTFOUND")
        message(STATUS "Found doom1.wad: ${DOOM1_WAD}")
        set(doom1wad_FOUND ON)
    endif()
endif()
