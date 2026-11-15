#include "i_video.h"

#include "doomgeneric_imp.h"
#include "imp_tables.h"

#include <stdlib.h>
#include <string.h>

void I_GetEvent(void);
void I_InitInput(void);

int usegamma = 0;

byte* I_VideoBuffer = NULL;

int usemouse = 0;
float mouse_acceleration = 2.0;
int mouse_threshold = 10;

boolean screenvisible = false;
boolean screensaver_mode = false;

static uint8_t s_colors[256];
static uint32_t* s_doom_screen_buffer = NULL;

void I_InitGraphics()
{
    // TODO - Reduce I_VideoBuffer to 128x96
    I_VideoBuffer = (byte*)malloc(SCREENWIDTH * SCREENHEIGHT); // For DOOM to draw on

    s_doom_screen_buffer = allocate_screen_buffer();
    screenvisible = true;

    I_InitInput();
}

void I_GraphicsCheckCommandLine()
{
}

void I_ShutdownGraphics()
{
    set_active_screen_buffer(NULL);

    // TODO - Free screen buffer

    free(I_VideoBuffer);
}

void I_SetPalette(uint8_t* palette)
{
    const uint32_t* palette32 = (const uint32_t*)palette;
    uint32_t* colors32 = (uint32_t*)s_colors;

    // Convert from RGB888 to RGB222
    for (int i = 0; i != 64; ++i)
    {
        uint32_t a = palette32[0];
        uint32_t b = palette32[1];
        uint32_t c = palette32[2];

        *colors32 =
            ((a & 0x000000C0) >> 2)  |  // -------- -------- -------- rr------ -> -------- -------- -------- --rr----
            ((a & 0x0000C000) >> 12) |  // -------- -------- gg------ -------- -> -------- -------- -------- ----gg--
            ((a & 0x00C00000) >> 22) |  // -------- bb------ -------- -------- -> -------- -------- -------- ------bb
            ((a & 0xC0000000) >> 18) |  // rr------ -------- -------- -------- -> -------- -------- --rr---- --------
            ((b & 0x000000C0) << 4)  |  // -------- -------- -------- gg------ -> -------- -------- ----gg-- --------
            ((b & 0x0000C000) >> 6)  |  // -------- -------- bb------ -------- -> -------- -------- ------bb --------
            ((b & 0x00C00000) >> 2)  |  // -------- rr------ -------- -------- -> -------- --rr---- -------- --------
            ((b & 0xC0000000) >> 12) |  // gg------ -------- -------- -------- -> -------- ----gg-- -------- --------
            ((c & 0x000000C0) << 10) |  // -------- -------- -------- bb------ -> -------- ------bb -------- --------
            ((c & 0x0000C000) << 14) |  // -------- -------- rr------ -------- -> --rr---- -------- -------- --------
            ((c & 0x00C00000) << 4)  |  // -------- gg------ -------- -------- -> ----gg-- -------- -------- --------
            ((c & 0xC0000000) >> 6);    // bb------ -------- -------- -------- -> ------bb -------- -------- --------

        palette32 += 3;
        colors32++;
    }
}

void I_FinishUpdate()
{
    static const uint8_t offsets[8] = {0, 3, 5, 7, 10, 13, 15, 17};
    uint32_t* dst = s_doom_screen_buffer;
    const byte* src = I_VideoBuffer;

    src += 4 * 320; // Skip the first 4 lines

    for (int y = 4; y != 196; y += 2)
    {
        // Each chunk of 20 pixels is encoded into a group of 8 pixels. Probably room for optimizations
        for (int x = 0; x != 320; x += 20)
        {
            uint8_t pixels[8];
            uint8_t chrominances[8];
            uint16_t total_chrominance = 0;
            uint8_t chrominance_count = 0;

            for (int i = 0; i != 8; ++i)
            {
                uint8_t c = s_colors[src[offsets[i]]];
                pixels[i] = c;
                if (c == 0)
                {
                    chrominances[i] = 0;
                }
                else
                {
                    uint8_t chrominance = g_chrominance_table[c];
                    chrominances[i] = chrominance;
                    total_chrominance += chrominance;
                    chrominance_count++;
                }
            }

            uint8_t avg_chrominance = chrominance_count == 0 ? 0 : total_chrominance / chrominance_count;

            uint8_t rs[3] = {0, 0, 0};
            uint8_t gs[3] = {0, 0, 0};
            uint8_t bs[3] = {0, 0, 0};
            uint8_t counts[3] = {0, 0};
            for (int dx = 0; dx != 8; ++dx)
            {
                uint8_t p = pixels[dx];
                if (p == 0)
                {
                    continue;
                }
                uint8_t r = p >> 4;
                uint8_t g = (p >> 2) & 0x03;
                uint8_t b = p & 0x03;

                uint8_t index = chrominances[dx] < avg_chrominance ? 0 : 1;

                rs[index] += r;
                gs[index] += g;
                bs[index] += b;
                counts[index]++;
                rs[2] += r;
                gs[2] += g;
                bs[2] += b;
                counts[2] += 2;
            }

            uint8_t colors[4];
            colors[0] = 0;
            for (int i = 0; i != 3; ++i)
            {
                uint8_t count = counts[i];
                if (count != 0)
                {
                    rs[i] /= count;
                    gs[i] /= count;
                    bs[i] /= count;
                }

                colors[i + 1] = (rs[i] << 4) | (gs[i] << 2) | bs[i];
            }

            uint32_t chunk = ((uint32_t)colors[2] << 24) | ((uint32_t)colors[1] << 16);
            for (int dx = 0; dx != 8; ++dx)
            {
                uint8_t p = pixels[dx];
                uint32_t c = 4;

                for (int i = 0; i != 4; ++i)
                {
                    if (colors[i] == p)
                    {
                        c = i;
                        break;
                    }
                }

                if (c == 4)
                {
                    c = 0;
                    uint16_t best_error = g_color_distance_table[(uint16_t)p << 6];
                    for (int i = 1; i != 3; ++i)
                    {
                        uint16_t error = g_color_distance_table[((uint16_t)p << 6) | colors[i]];
                        if (error < best_error)
                        {
                            c = i;
                            best_error = error;
                        }
                    }
                }

                chunk |= c << (dx * 2);
            }

            *dst++ = chunk;

            src += 20;
        }

        src += 320;
    }
    
    set_active_screen_buffer(s_doom_screen_buffer);
}

int I_GetPaletteIndex(int r, int g, int b)
{
    (void)r;
    (void)g;
    (void)b;
    // Only used for mouse
    return 0;
}

void I_UpdateNoBlit()
{
}

void I_ReadScreen(byte* scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

void I_BeginRead()
{
}

void I_SetWindowTitle(char* title)
{
    (void)title;
}

void I_CheckIsScreensaver()
{
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    (void)func;
}

void I_InitWindowTitle()
{
}

void I_InitWindowIcon()
{
}

void I_StartFrame()
{
}

void I_StartTic()
{
    I_GetEvent();
}

void I_EnableLoadingDisk()
{
}

void I_EndRead()
{
}

void I_BindVideoVariables()
{
}

void I_DisplayFPSDots(boolean dots_on)
{
    (void)dots_on;
}
