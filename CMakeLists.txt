cmake_minimum_required(VERSION 3.13)

project(tt-doom LANGUAGES C)

find_package(X11)
find_package(SDL2)
find_package(SDL2_mixer)

add_library(doom-base INTERFACE)
target_sources(doom-base INTERFACE
    src/dummy.c
    src/am_map.c
    src/doomdef.c
    src/doomstat.c
    src/dstrings.c
    src/d_event.c
    src/d_items.c
    src/d_iwad.c
    src/d_loop.c
    src/d_main.c
    src/d_mode.c
    src/d_net.c
    src/f_finale.c
    src/f_wipe.c
    src/g_game.c
    src/hu_lib.c
    src/hu_stuff.c
    src/info.c
    src/i_cdmus.c
    src/i_endoom.c
    src/i_joystick.c
    src/i_scale.c
    src/i_sound.c
    src/i_system.c
    src/i_timer.c
    src/memio.c
    src/m_argv.c
    src/m_bbox.c
    src/m_cheat.c
    src/m_config.c
    src/m_controls.c
    src/m_fixed.c
    src/m_menu.c
    src/m_misc.c
    src/m_random.c
    src/p_ceilng.c
    src/p_doors.c
    src/p_enemy.c
    src/p_floor.c
    src/p_inter.c
    src/p_lights.c
    src/p_map.c
    src/p_maputl.c
    src/p_mobj.c
    src/p_plats.c
    src/p_pspr.c
    src/p_saveg.c
    src/p_setup.c
    src/p_sight.c
    src/p_spec.c
    src/p_switch.c
    src/p_telept.c
    src/p_tick.c
    src/p_user.c
    src/r_bsp.c
    src/r_data.c
    src/r_draw.c
    src/r_main.c
    src/r_plane.c
    src/r_segs.c
    src/r_sky.c
    src/r_things.c
    src/sha1.c
    src/sounds.c
    src/statdump.c
    src/st_lib.c
    src/st_stuff.c
    src/s_sound.c
    src/tables.c
    src/v_video.c
    src/wi_stuff.c
    src/w_checksum.c
    src/w_file.c
    src/w_main.c
    src/w_wad.c
    src/z_zone.c
    src/w_file_stdc.c
    src/i_input.c
    src/i_video.c
    src/doomgeneric.c
)

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

add_executable(doomgeneric-linuxvt
    src/doomgeneric_linuxvt.c
    src/mus2mid.c
)
target_compile_definitions(doomgeneric-linuxvt PRIVATE NORMALUNIX LINUX SNDSERV _DEFAULT_SOURCE)
target_link_libraries(doomgeneric-linuxvt PRIVATE doom-base)

if(WIN32)
    add_executable(doomgeneric-win
        src/doomgeneric_win.c
    )
    target_compile_definitions(doomgeneric-win PRIVATE SNDSERV _DEFAULT_SOURCE)
    target_link_libraries(doomgeneric-win PRIVATE gdi32)
endif()
