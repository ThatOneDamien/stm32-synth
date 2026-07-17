/* STM32F446RETx linker script */

ENTRY(Reset_Handler)

/*
 * Memory layout
 *
 * STM32F446RE:
 *   Flash: 512 KB @ 0x08000000
 *   SRAM:  128 KB @ 0x20000000
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}

/*
 * Stack starts at the top of RAM
 * and grows downward.
 */
_estack = ORIGIN(RAM) + LENGTH(RAM);


SECTIONS
{
    /*
     * Vector table + program code
     */
    .text :
    {
        . = ALIGN(4);

        KEEP(*(.isr_vector))

        *(.text)
        *(.text.*)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);

        _etext = .;

    } > FLASH


    /*
     * Read-only data
     */
    .rodata :
    {
        . = ALIGN(4);

        *(.rodata)
        *(.rodata.*)

        . = ALIGN(4);

    } > FLASH


    /*
     * Initialized variables
     *
     * Stored in Flash, copied to RAM by Reset_Handler.
     */
    .data :
    {
        . = ALIGN(4);

        _sdata = .;

        *(.data)
        *(.data.*)

        . = ALIGN(4);

        _edata = .;

    } > RAM AT > FLASH


    /*
     * Location of the initial .data values in Flash.
     *
     * Reset_Handler copies:
     *
     *   _sidata --> _sdata
     */
    _sidata = LOADADDR(.data);


    /*
     * Zero initialized variables
     *
     * Reset_Handler clears this region.
     */
    .bss :
    {
        . = ALIGN(4);

        _sbss = .;

        *(.bss)
        *(.bss.*)

        *(COMMON)

        . = ALIGN(4);

        _ebss = .;

    } > RAM


    /*
     * Reserve stack space.
     *
     * The actual stack pointer starts at _estack.
     */
    .stack :
    {
        . = ALIGN(8);

        . = . + 4K;

        . = ALIGN(8);

    } > RAM
}
