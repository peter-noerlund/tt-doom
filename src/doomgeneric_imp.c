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

static volatile uint32_t* s_devices = (volatile uint32_t*)0x2000000;
static volatile uint32_t s_tick_count = 0;
static uint32_t* s_console_screen_buffer = NULL;
static uint32_t* s_active_screen_buffer = NULL;
static int s_console_line = 0;
static int s_console_column = 0;

void interrupt_handler()
{
    ++s_tick_count;

    s_devices[DEVICE_TIMER] = 65000;
}

inline void set_csr_flags(uint16_t csr, uint32_t flags)
{
    __asm__ volatile ("csrs %0, %1" : : "i"(csr), "r"(flags));
}

uint32_t* allocate_screen_buffer(void)
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

void set_active_screen_buffer(uint32_t* screen_buffer)
{
    if (s_active_screen_buffer == screen_buffer)
    {
        return;
    }

    if (screen_buffer == NULL)
    {
        s_devices[DEVICE_VIDEO] = 0;
    }
    else
    {
        s_devices[DEVICE_VIDEO] = (uint32_t)screen_buffer | 0x80000000;
    }

    s_active_screen_buffer = screen_buffer;
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
            s_console_screen_buffer[y * 16 + x] = s_console_screen_buffer[(y + 6) * 16 + x];
        }
    }
    for (int y = 90; y != 96; ++y)
    {
        for (int x = 0; x != 16; ++x)
        {
            s_console_screen_buffer[y * 16 + x] = 0x003F0000;
        }
    }
}

static int console_putc(char c, FILE* file)
{
    (void)file;

    set_active_screen_buffer(s_console_screen_buffer);

    if ((c & 0x80) != 0)
    {
        return 0;
    }

    if (c == '\n')
    {
        s_console_line++;
        s_console_column = 0;
        if (s_console_line == 16)
        {
            console_scroll();
            s_console_line = 15;
        }
        return 0;
    }

    if (s_console_column == 32)
    {
        s_console_column = 0;
        s_console_line++;
        if (s_console_line == 16)
        {
            console_scroll();
            s_console_line = 15;
        }
    }

    volatile uint8_t* console = (volatile uint8_t*)s_console_screen_buffer;
    const uint8_t* glyph = font_imp[(unsigned char)c & 0x7F];
    for (int row = 0; row != 6; ++row)
    {
        console[(s_console_line * 6 + row) * 64 + (s_console_column / 2) * 4 + (s_console_column % 2)] = glyph[row];
    }
    s_console_column++;

    return 0;
}

static void console_init(void)
{
    s_console_screen_buffer = allocate_screen_buffer();

    volatile uint32_t* ptr = s_console_screen_buffer;
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

#ifdef TT_ECALL_QUIT
    __asm__ volatile (
            "mv a1, %0\n"
            "mv a0, zero\n"
            "ecall" : : "r"(status));
#else // TT_ECALL_QUIT
    (void)status;
#endif // TT_ECALL_QUIT
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

__attribute__((naked)) int __mulsi3(int a, int b)
{
    __asm__(
        // Determine result sign
        "xor a4, a0, a1\n"

        // Absolute values
        "srai t0, a0, 31\n"
        "xor a0, a0, t0\n"
        "sub a0, a0, t0\n"
        "srai t0, a1, 31\n"
        "xor a1, a1, t0\n"
        "sub a1, a1, t0\n"

        // Swap for optimization
        "bgeu a0, a1, .Lmulsi3Init\n"
        "mv t0, a0\n"
        "mv a0, a1\n"
        "mv a1, t0\n"

        // Loop setup
        ".Lmulsi3Init:\n"
        "mv a2, a0\n"
        "li a0, 0\n"

        // Loop
        ".Lmulsi3Loop:\n"
        "andi a3, a1, 1\n"
        "beqz a3, .Lmulsi3Shift\n"
        "add a0, a0, a2\n"
        ".Lmulsi3Shift:\n"
        "srli a1, a1, 1\n"
        "slli a2, a2, 1\n"
        "bnez a1, .Lmulsi3Loop\n"

        // Restore sign
        "bgez a4, .Lmulsi3Done\n"
        "not a0, a0\n"
        "addi a0, a0, 1\n"

        ".Lmulsi3Done:\n"
        "ret\n"
    );
}



// DOOM stuff

void DG_Init()
{
    // Enable timer interrupts
    set_csr_flags(CSR_MTVEC, 0x00000004);       // Just for show, it is hardcoded in hardware.
    set_csr_flags(CSR_MIE, MIE_TIMER);          // Enable timer interrupt
    set_csr_flags(CSR_MSTATUS, MSTATUS_MIE);    // Enable interrupts globally

    s_devices[DEVICE_TIMER] = 65000; // Run timer at 1ms interval

    // Frame buffer is actually 128x96, but we run at 128x80
}

void DG_DrawFrame()
{
}

void DG_SleepMs(uint32_t ms)
{
    uint32_t current_tick_count = s_tick_count;
    uint32_t target_tick_count = current_tick_count + ms;

    if (target_tick_count < current_tick_count)
    {
        // Timer overflows, so start by waiting until it overflows
        while (s_tick_count > current_tick_count);
    }

    while (s_tick_count < target_tick_count);
}

uint32_t DG_GetTicksMs()
{
    return (uint32_t)s_tick_count;
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

#if defined(TT_BENCHMARK)
    char* argv[4] = {"doomgeneric-imp", "-timedemo", TT_BENCHMARK_DEMO, NULL};
    int argc = 4;
#else // TT_TIMEDEMO1 || TT_TIMEDEMO2 || TT_TIMEDEMO3
    char** argv = NULL;
    int argc = 0;
#endif // TT_TIMEDEMO1 || TT_TIMEDEMO2 || TT_TIMEDEMO3

    doomgeneric_Create(argc, argv);

    while (1)
    {
        doomgeneric_Tick();
    }

    return 0;
}


