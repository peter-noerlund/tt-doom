#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "doomgeneric.h"
#include "font_imp.h"

#define DEVICE_TIMER    0
#define DEVICE_INPUT    1
#define DEVICE_AUDIO    2
#define DEVICE_VIDEO    3

#define CSR_MSTATUS 0x300
#define CSR_MIE 0x304
#define CSR_MTVEC 0x305

#define MSTATUS_MIE 0x8

#define MIE_TIMER 0x80

#define VIDEO_BUFFER_ALIGNMENT (64 * 128)

#define SCREEN_BUFFER_NONE 0
#define SCREEN_BUFFER_CONSOLE 1
#define SCREEN_BUFFER_DOOM 2

static volatile uint32_t* g_devices = (volatile uint32_t*)0x2000000;
static volatile uint32_t g_tick_count = 0;
static volatile uint32_t* g_doom_screen_buffer = NULL;
static volatile uint32_t* g_console_screen_buffer = NULL;
static int g_screen_buffer = SCREEN_BUFFER_NONE;
static int g_console_line = 0;
static int g_console_column = 0;

void interrupt_handler()
{
    ++g_tick_count;

    g_devices[DEVICE_TIMER] = 65000;
}

inline void set_csr_flags(uint16_t csr, uint32_t flags)
{
    __asm__ volatile ("csrs %0, %1" : : "i"(csr), "r"(flags));
}

static volatile uint32_t* allocate_screen_buffer(void)
{
    // Allocate screen buffer
    uint32_t* unaligned_buffer = (uint32_t*)calloc(16 * 128 * 2, 4);
    uintptr_t ptr = (uintptr_t)unaligned_buffer;
    if ((ptr % VIDEO_BUFFER_ALIGNMENT) != 0)
    {
        ptr += VIDEO_BUFFER_ALIGNMENT - (ptr % VIDEO_BUFFER_ALIGNMENT);
    }

    return (uint32_t*)ptr;
}

static void set_screen_buffer(int screen_buffer)
{
    if (g_screen_buffer == screen_buffer)
    {
        return;
    }

    if (screen_buffer == SCREEN_BUFFER_DOOM)
    {
        g_devices[DEVICE_VIDEO] = (uint32_t)g_doom_screen_buffer | 0x80000000;
    }
    else if (screen_buffer == SCREEN_BUFFER_CONSOLE)
    {
        g_devices[DEVICE_VIDEO] = (uint32_t)g_console_screen_buffer | 0x80000000;
    }
    else
    {
        g_devices[DEVICE_VIDEO] = 0;
    }

    g_screen_buffer = screen_buffer;
}

static int console_getc(FILE* file)
{
    (void)file;
    return 0;
}

static void console_scroll(void)
{
    for (int y = 0; y != 90; ++y)
    {
        for (int x = 0; x != 16; ++x)
        {
            g_console_screen_buffer[y * 16 + x] = g_console_screen_buffer[(y + 6) * 16 + x];
        }
    }
    for (int y = 90; y != 96; ++y)
    {
        for (int x = 0; x != 16; ++x)
        {
            g_console_screen_buffer[y * 16 + x] = 0x003F0000;;
        }
    }
}

static int console_putc(char c, FILE* file)
{
    (void)file;

    set_screen_buffer(SCREEN_BUFFER_CONSOLE);

    if ((c & 0x80) != 0)
    {
        return 0;
    }

    if (c == '\n')
    {
        g_console_line++;
        g_console_column = 0;
        if (g_console_line == 16)
        {
            console_scroll();
            g_console_line = 15;
        }
        return 0;
    }

    if (g_console_column == 32)
    {
        g_console_column = 0;
        g_console_line++;
        if (g_console_line == 16)
        {
            console_scroll();
            g_console_line = 15;
        }
    }

    volatile uint8_t* console = (volatile uint8_t*)g_console_screen_buffer;
    const uint8_t* glyph = font_imp[(unsigned char)c & 0x7F];
    for (int row = 0; row != 6; ++row)
    {
        console[(g_console_line * 6 + row) * 64 + (g_console_column / 2) * 4 + (g_console_column % 2)] = glyph[row];
    }
    g_console_column++;

    return 0;
}

static void console_init(void)
{
    g_console_screen_buffer = allocate_screen_buffer();

    volatile uint32_t* ptr = g_console_screen_buffer;
    for (int y = 0; y != 96; ++y)
    {
        for (int x = 0; x != 16; ++x)
        {
            *ptr++ = 0x003F0000;
        }
    }
}

static FILE console_stdin = FDEV_SETUP_STREAM(NULL, console_getc, NULL, _FDEV_SETUP_READ);
static FILE console_stdout = FDEV_SETUP_STREAM(console_putc, NULL, NULL, _FDEV_SETUP_WRITE);

FILE* const stdin = &console_stdin;
FILE* const stdout = &console_stdout;
FILE* const stderr = &console_stdout;

void _exit(int status)
{
    (void)status;
    for (;;);
}

int unlink(char* path)
{
    (void)path;
    errno = ENOENT;
    return -1;
}

int rename(const char* oldpath, const char* newpath)
{
    (void)oldpath;
    (void)newpath;
    errno = ENOENT;
    return -1;
}

int open(const char* path, int flags, ...)
{
    (void)path;
    (void)flags;
    errno = ENOENT;
    return -1;
}

ssize_t read(int fd, void* buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;
    errno = EBADF;
    return -1;
}

ssize_t write(int fd, const void* data, size_t count)
{
    (void)fd;
    (void)data;
    (void)count;
    errno = EBADF;
    return -1;
}

off_t lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    errno = EBADF;
    return -1;
}

int close(int fd)
{
    (void)fd;
    errno = EBADF;
    return -1;
}

// DOOM stuff

void DG_Init()
{
    // Enable timer interrupts
    set_csr_flags(CSR_MTVEC, 0x00000004);       // Just for show, it is hardcoded in hardware.
    set_csr_flags(CSR_MIE, MIE_TIMER);          // Enable timer interrupt
    set_csr_flags(CSR_MSTATUS, MSTATUS_MIE);    // Enable interrupts globally

    g_devices[DEVICE_TIMER] = 65000; // Run timer at 1ms interval

    // Frame buffer is actually 128x96, but we run at 128x80
    g_doom_screen_buffer = allocate_screen_buffer();
}

static uint8_t calculate_chrominance(uint8_t r, uint8_t g, uint8_t b)
{
    return r * 3 + g * 7 + b;
}

inline uint8_t diff(uint8_t a, uint8_t b)
{
    return a < b ? b - a : a - b;
}

static uint32_t color_distance(uint8_t c1, uint8_t c2)
{
    uint8_t r1 = c1 >> 4;
    uint8_t r2 = c2 >> 4;
    uint8_t g1 = (c1 >> 2) & 0x03;
    uint8_t g2 = (c2 >> 2) & 0x03;
    uint8_t b1 = c1 & 0x03;
    uint8_t b2 = c2 & 0x03;

    uint8_t rd = diff(r1, r2);
    uint8_t gd = diff(g1, g2);
    uint8_t bd = diff(b1, b2);

    return (uint32_t)rd * rd + (uint32_t)gd * gd + (uint32_t)bd * bd;

}

void DG_DrawFrame()
{
    static const uint8_t offsets[8] = {0, 3, 5, 7, 10, 13, 15, 17};
    static const uint8_t encoding[16] = {
        0,  // 0 0 
        1,  // 1 0
        2,  // 2 0
        0,  // invalid
        
        3,  // 1 0
        4,  // 1 1
        5,  // 1 2
        0,  // invalid
        
        6,  // 2 0
        5,  // 2 1 - invalid, use 5 instead for now
        7,  // 2 2
        0,  // invalid

        0,  // invalid
        0,  // invalid
        0,  // invalid
        0   // invalid
    };
    volatile uint32_t* dst = g_doom_screen_buffer;
    const pixel_t* src = DG_ScreenBuffer;

    for (int y = 4; y < 196; y += 2)
    {
        for (int x = 0; x < 320; x += 20)
        {
            uint8_t pixels[8];
            uint8_t chrominances[8];
            uint16_t total_chrominance = 0;
            uint8_t chrominance_count = 0;

            for (int i = 0; i != 8; ++i)
            {
                uint32_t input_color = src[offsets[i]];
                uint8_t r = (input_color >> (16 + 6)) & 0x03;
                uint8_t g = (input_color >> (8 + 6)) & 0x03;
                uint8_t b = (input_color >> 6) & 0x03;
                uint8_t c = (r << 4) | (g << 2) | b;
                pixels[i] = c;
                if (c == 0)
                {
                    chrominances[i] = 0;
                }
                else
                {
                    uint8_t chrominance = calculate_chrominance(r, g, b);
                    chrominances[i] = chrominance;
                    total_chrominance += chrominance;
                    chrominance_count++;
                }
            }

            uint8_t avg_chrominance = chrominance_count == 0 ? 0 : total_chrominance / chrominance_count;

            uint8_t rs[2] = {0, 0};
            uint8_t gs[2] = {0, 0};
            uint8_t bs[2] = {0, 0};
            uint8_t counts[2] = {0, 0};
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
            }

            uint8_t colors[3];
            colors[0] = 0;
            for (int i = 0; i != 2; ++i)
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

            uint32_t chunk = 0;
            for (int dx = 0; dx != 8; dx += 2)
            {
                uint8_t mask = 0;
                for (int i = 0; i != 2; ++i)
                {
                    if (pixels[dx + i] == 0)
                    {
                        continue;
                    }
                    else if (chrominances[dx + i] < avg_chrominance)
                    {
                        mask |= 1 << (i * 2);
                    }
                    else
                    {
                        mask |= 2 << (i * 2);
                    }
                }

                if (mask == 0x9) // 10 01
                {
                    uint8_t p1 = pixels[dx];
                    uint8_t p2 = pixels[dx + 1];

                    mask = 0;
                    uint32_t best_error = color_distance(p1, 0) + color_distance(p2, 0);
                    for (int i1 = 0; i1 != 3; ++i1)
                    {
                        for (int i2 = 0; i2 != 3; ++i2)
                        {
                            if ((i1 == 0 && i2 == 0) || (i1 == 2 && i2 == 1))
                            {
                                continue;
                            }

                            uint32_t error = color_distance(p1, colors[i1]) + color_distance(p2, colors[i2]);
                            if (error < best_error)
                            {
                                best_error = error;
                                mask = i2 << 2 | i1;
                            }
                        }
                    }
                }

                chunk |= (uint32_t)encoding[mask] << (dx * 2);
            }

            chunk |= ((uint32_t)colors[2] << 24) | ((uint32_t)colors[1] << 16);
            //chunk |= 0x300C0000;

            *dst++ = chunk;

            /*
            if (y == 104 && x == 60)
            {
                printf("pix [%02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX]\n", pixels[0], pixels[1], pixels[2], pixels[3], pixels[4], pixels[5], pixels[6], pixels[7]);
                printf("chr [%02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX]\n", chrominances[0], chrominances[1], chrominances[2], chrominances[3], chrominances[4], chrominances[5], chrominances[6], chrominances[7]);
                printf("avg %02hhX\n", avg_chrominance);
                printf("cnt [%02hhX %02hhX]\n", counts[0], counts[1]);
                printf("col [%02hhX %02hhX %02hhX]\n", colors[0], colors[1], colors[2]);
                printf("res %08X\n", chunk);
                puts("");
            }
            */

            src += 20;
        }

        src += 320;
    }
    
    set_screen_buffer(SCREEN_BUFFER_DOOM);
}

void DG_SleepMs(uint32_t ms)
{
    uint32_t current_tick_count = g_tick_count;
    uint32_t target_tick_count = current_tick_count + ms;

    if (target_tick_count < current_tick_count)
    {
        // Timer overflows, so start by waiting until it overflows
        while (g_tick_count > current_tick_count);
    }

    while (g_tick_count < target_tick_count);
}

uint32_t DG_GetTicksMs()
{
    return (uint32_t)g_tick_count;
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
    (void)pressed;
    (void)doomKey;
    // TODO
    return 0;
}

void DG_SetWindowTitle(const char* title)
{
    (void)title;
}

// Entry

int main()
{
    console_init();

    doomgeneric_Create(0, NULL);

    while (1)
    {
        doomgeneric_Tick();
    }

    return 0;
}


