#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "doomgeneric.h"

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
        g_devices[DEVICE_VIDEO] = (uint32_t)g_console_screen_buffer | 0x8000000;
    }
    else
    {
        g_devices[DEVICE_VIDEO] = 0;
    }
}

static int console_getc(FILE* file)
{
    (void)file;
    return 0;
}

static int console_putc(char c, FILE* file)
{
    (void)file;

    set_screen_buffer(SCREEN_BUFFER_CONSOLE);

    // TODO

    return 0;
}

static void console_init(void)
{
    g_console_screen_buffer = allocate_screen_buffer();

    volatile uint32_t* ptr = g_console_screen_buffer;
    for (int y = 0; y != 16; ++y)
    {
        for (int x = 0; x != 16; ++x)
        {
            *ptr++ = 0x00003F00;
        }
    }
}

static FILE console_stdin = FDEV_SETUP_STREAM(NULL, console_getc, NULL, _FDEV_SETUP_READ);
static FILE console_stdout = FDEV_SETUP_STREAM(console_putc, NULL, NULL, _FDEV_SETUP_WRITE);

FILE* const stdin = &console_stdin;
FILE* const stdout = &console_stdout;
FILE* const stderr = &console_stdout;

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

void DG_DrawFrame()
{
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

// libc

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
