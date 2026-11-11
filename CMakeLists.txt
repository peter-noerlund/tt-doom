cmake_minimum_required(VERSION 3.13)

project(tt-doom LANGUAGES C)

find_package(X11)
find_package(SDL2)
find_package(SDL2_mixer)

add_library(doomgeneric-base INTERFACE)
target_sources(doomgeneric-base INTERFACE
    doomgeneric/dummy.c
    doomgeneric/am_map.c
    doomgeneric/doomdef.c
    doomgeneric/doomstat.c
    doomgeneric/dstrings.c
    doomgeneric/d_event.c
    doomgeneric/d_items.c
    doomgeneric/d_iwad.c
    doomgeneric/d_loop.c
    doomgeneric/d_main.c
    doomgeneric/d_mode.c
    doomgeneric/d_net.c
    doomgeneric/f_finale.c
    doomgeneric/f_wipe.c
    doomgeneric/g_game.c
    doomgeneric/hu_lib.c
    doomgeneric/hu_stuff.c
    doomgeneric/info.c
    doomgeneric/i_cdmus.c
    doomgeneric/i_endoom.c
    doomgeneric/i_joystick.c
    doomgeneric/i_scale.c
    doomgeneric/i_sound.c
    doomgeneric/i_system.c
    doomgeneric/i_timer.c
    doomgeneric/memio.c
    doomgeneric/m_argv.c
    doomgeneric/m_bbox.c
    doomgeneric/m_cheat.c
    doomgeneric/m_config.c
    doomgeneric/m_controls.c
    doomgeneric/m_fixed.c
    doomgeneric/m_menu.c
    doomgeneric/m_misc.c
    doomgeneric/m_random.c
    doomgeneric/p_ceilng.c
    doomgeneric/p_doors.c
    doomgeneric/p_enemy.c
    doomgeneric/p_floor.c
    doomgeneric/p_inter.c
    doomgeneric/p_lights.c
    doomgeneric/p_map.c
    doomgeneric/p_maputl.c
    doomgeneric/p_mobj.c
    doomgeneric/p_plats.c
    doomgeneric/p_pspr.c
    doomgeneric/p_saveg.c
    doomgeneric/p_setup.c
    doomgeneric/p_sight.c
    doomgeneric/p_spec.c
    doomgeneric/p_switch.c
    doomgeneric/p_telept.c
    doomgeneric/p_tick.c
    doomgeneric/p_user.c
    doomgeneric/r_bsp.c
    doomgeneric/r_data.c
    doomgeneric/r_draw.c
    doomgeneric/r_main.c
    doomgeneric/r_plane.c
    doomgeneric/r_segs.c
    doomgeneric/r_sky.c
    doomgeneric/r_things.c
    doomgeneric/sha1.c
    doomgeneric/sounds.c
    doomgeneric/statdump.c
    doomgeneric/st_lib.c
    doomgeneric/st_stuff.c
    doomgeneric/s_sound.c
    doomgeneric/tables.c
    doomgeneric/v_video.c
    doomgeneric/wi_stuff.c
    doomgeneric/w_checksum.c
    doomgeneric/w_file.c
    doomgeneric/w_main.c
    doomgeneric/w_wad.c
    doomgeneric/z_zone.c
    doomgeneric/w_file_stdc.c
    doomgeneric/i_input.c
    doomgeneric/i_video.c
    doomgeneric/doomgeneric.c
)

if(X11_FOUND)
    add_executable(doomgeneric-xlib
        doomgeneric/doomgeneric_xlib.c
    )
    target_compile_definitions(doomgeneric-xlib PRIVATE NORMALUNIX LINUX SNDSERV _DEFAULT_SOURCE)
    target_link_libraries(doomgeneric-xlib PRIVATE doomgeneric-base X11::X11)
endif()

if(SDL2_FOUND AND SDL2_mixer_FOUND)
    add_executable(doomgeneric-sdl
        doomgeneric/doomgeneric_sdl.c
        doomgeneric/mus2mid.c
        doomgeneric/i_sdlmusic.c
        doomgeneric/i_sdlsound.c
    )
    target_compile_definitions(doomgeneric-sdl PRIVATE FEATURE_SOUND)
    target_link_libraries(doomgeneric-sdl PRIVATE doomgeneric-base SDL2::SDL2 SDL2_mixer::SDL2_mixer)
endif()

add_executable(doomgeneric-linuxvt
    doomgeneric/doomgeneric_linuxvt.c
    doomgeneric/mus2mid.c
)
target_compile_definitions(doomgeneric-linuxvt PRIVATE NORMALUNIX LINUX SNDSERV _DEFAULT_SOURCE)
target_link_libraries(doomgeneric-linuxvt PRIVATE doomgeneric-base)

if(WIN32)
    add_executable(doomgeneric-win
        doomgeneric/doomgeneric_win.c
    )
    target_compile_definitions(doomgeneric-win PRIVATE SNDSERV _DEFAULT_SOURCE)
    target_link_libraries(doomgeneric-win PRIVATE gdi32)
endif()
