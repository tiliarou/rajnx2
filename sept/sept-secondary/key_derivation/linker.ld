OUTPUT_ARCH(aarch64)

ENTRY(_start)
SECTIONS
{
    . = 0x4003D000;

    __start__ = ABSOLUTE(.);

    .text   : ALIGN(4) { *(.text.start) *(.text*); . = ALIGN(4); }
    .rodata : ALIGN(4) { *(.rodata*); . = ALIGN(4); }
    .bss    : ALIGN(8) { __bss_start__ = .; *(.bss* COMMON); . = ALIGN(8); __bss_end__ = .; }

    . = ALIGN(4);

    __end__ = ABSOLUTE(.);
}