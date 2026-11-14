cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR})

project(tt-doom LANGUAGES C)

option(TT_BENCHMARK "Run benchmark" OFF)
option(TT_ECALL_QUIT "Call ecall on quit" OFF)

set(TT_BENCHMARK_DEMO "demo1" CACHE STRING "Benchmark demo")

find_package(doom1wad CONFIG REQUIRED)
find_package(riscv-compiler CONFIG)
find_package(riscv-picolibc CONFIG)
find_package(SDL2)
find_package(SDL2_mixer)
find_package(X11)

set(DOOM_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dummy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/am_map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/doomdef.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/doomstat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dstrings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/d_event.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/d_items.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/d_iwad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/d_loop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/d_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/d_mode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/d_net.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/f_finale.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/f_wipe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/g_game.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hu_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hu_stuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/info.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_cdmus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_endoom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_joystick.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_scale.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_sound.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_system.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_timer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/memio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_argv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_bbox.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_cheat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_config.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_controls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_fixed.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_menu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/m_random.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_ceilng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_doors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_enemy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_floor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_inter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_lights.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_maputl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_mobj.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_plats.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_pspr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_saveg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_setup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_sight.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_spec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_switch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_telept.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_tick.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/p_user.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/r_bsp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/r_data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/r_draw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/r_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/r_plane.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/r_segs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/r_sky.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/r_things.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sha1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sounds.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/statdump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/st_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/st_stuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/s_sound.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tables.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/v_video.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wi_stuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/w_checksum.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/w_file.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/w_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/w_wad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/z_zone.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/w_file_stdc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_input.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_video.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/doomgeneric.c
    ${CMAKE_CURRENT_BINARY_DIR}/doom1_wad.c
)

add_library(doom-base INTERFACE)
target_sources(doom-base INTERFACE ${DOOM_SOURCES})
target_include_directories(doom-base INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(file-to-c tools/file_to_c.c)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(file-to-c PRIVATE -Wall -W -Wextra -Wpedantic)
endif()

add_executable(encode-font tools/encode_font.c)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(encode-font PRIVATE -Wall -W -Wextra -Wpedantic)
endif()
target_include_directories(encode-font PRIVATE tools)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doom1_wad.c
    COMMAND $<TARGET_FILE:file-to-c> ${DOOM1_WAD} ${CMAKE_CURRENT_BINARY_DIR}/doom1_wad.c ${CMAKE_CURRENT_BINARY_DIR}/doom1_wad.h doom1_wad
    DEPENDS ${DOOM1WAD} $<TARGET_FILE:file-to-c>
)

add_executable(doomgeneric-linuxvt
    src/doomgeneric_linuxvt.c
    src/mus2mid.c
)
target_compile_definitions(doomgeneric-linuxvt PRIVATE NORMALUNIX LINUX SNDSERV _DEFAULT_SOURCE)
target_link_libraries(doomgeneric-linuxvt PRIVATE doom-base)

if(X11_FOUND)
    add_executable(doomgeneric-xlib
        src/doomgeneric_xlib.c
    )
    target_compile_definitions(doomgeneric-xlib PRIVATE NORMALUNIX LINUX SNDSERV _DEFAULT_SOURCE)
    target_link_libraries(doomgeneric-xlib PRIVATE doom-base X11::X11)
endif()

if(SDL2_FOUND AND SDL2_mixer_FOUND)
    add_executable(doomgeneric-sdl
        src/doomgeneric_sdl.c
        src/mus2mid.c
        src/i_sdlmusic.c
        src/i_sdlsound.c
    )
    target_compile_definitions(doomgeneric-sdl PRIVATE FEATURE_SOUND)
    target_link_libraries(doomgeneric-sdl PRIVATE doom-base SDL2::SDL2 SDL2_mixer::SDL2_mixer)
endif()

if(WIN32)
    add_executable(doomgeneric-win
        src/doomgeneric_win.c
    )
    target_compile_definitions(doomgeneric-win PRIVATE SNDSERV _DEFAULT_SOURCE)
    target_link_libraries(doomgeneric-win PRIVATE gdi32)
endif()

if(riscv-compiler_FOUND AND riscv-picolibc_FOUND)
    set(DOOM_SOURCES_IMP ${DOOM_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/doomgeneric_imp.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/init_imp.S
        ${CMAKE_CURRENT_BINARY_DIR}/font_imp.c
    )

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/font_imp.c
        COMMAND $<TARGET_FILE:encode-font> ${CMAKE_CURRENT_BINARY_DIR}/font_imp.c ${CMAKE_CURRENT_BINARY_DIR}/font_imp.h font_imp
        DEPENDS $<TARGET_FILE:encode-font>
    )

    set(TT_EXTRA_ARGS "")
    if (TT_BENCHMARK)
        set(TT_EXTRA_ARGS ${TT_EXTRA_ARGS} -DTT_BENCHMARK "-DTT_BENCHMARK_DEMO=\\\"${TT_BENCHMARK_DEMO}\\\"")
    endif()
    if (TT_ECALL_QUIT)
        set(TT_EXTRA_ARGS ${TT_EXTRA_ARGS} -DTT_ECALL_QUIT)
    endif()

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doomgeneric-imp.elf
        COMMAND
            ${RISCV_GCC}
            -march=rv32ezicsr
            -mabi=ilp32e
            -mstrict-align
            -mcmodel=medany
            -mtune=size
            -mno-relax
            -Wl,--no-relax
            -fno-builtin
            -O3
            -Wall
            -Wno-format
            -Wno-unused-but-set-variable
            -Wno-unused-const-variable
            -o ${CMAKE_CURRENT_BINARY_DIR}/doomgeneric-imp.elf
            --specs=${RISCV_PICOLIBC}
            -T${CMAKE_CURRENT_SOURCE_DIR}/src/doomgeneric_imp.ld
            -DNDEBUG
            -D_DEFAULT_SOURCE
            -DDOOMGENERIC_RESX=320
            -DDOOMGENERIC_RESY=200
            ${TT_EXTRA_ARGS}
            -DTT_IMP
            -I ${CMAKE_CURRENT_SOURCE_DIR}/src
            -I ${CMAKE_CURRENT_BINARY_DIR}
            ${DOOM_SOURCES_IMP}
        DEPENDS ${DOOM_SOURCES_IMP} ${CMAKE_CURRENT_SOURCE_DIR}/src/doomgeneric_imp.ld ${CMAKE_CURRENT_BINARY_DIR}/font_imp.c
    )

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doomgeneric-imp.bin
        COMMAND ${RISCV_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/doomgeneric-imp.elf ${CMAKE_CURRENT_BINARY_DIR}/doomgeneric-imp.bin
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/doomgeneric-imp.elf
    )

    add_custom_target(doomgeneric-imp ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/doomgeneric-imp.bin
    )
endif()
