__flash = 0x8;
__flash_size = 16376K;
__ram = 0x01000800;
__ram_size = 8180K;
__stack_size = 512;

PHDRS
{
    vectors PT_LOAD FLAGS(5);
    scratch PT_LOAD FLAGS(6);
}

SECTIONS
{
    .vectors 0x0 :
    {
        KEEP(*(.vectors))
        . = 0x08;
    } :vectors

    .scratch 0x01000000 (NOLOAD) :
    {
       *(.scratch)
       . = 0x800;
    } :scratch
}

INCLUDE picolibc.ld
