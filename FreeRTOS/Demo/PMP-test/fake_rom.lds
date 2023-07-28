OUTPUT_ARCH( "riscv" )
ENTRY( _start )

MEMORY
{
	/* Fake ROM area */
	rom (rxa) : ORIGIN = 0x80000000, LENGTH = 512K
	ram (wxa) : ORIGIN = 0x80080000, LENGTH = 448K
    ram2 (wxa) : ORIGIN = 0x800e0000, LENGTH = 64K
}

/* PMP sections. */
__privileged_functions_region_size__  = 32K; /* 32000 bytes */
__privileged_data_region_size__       = 64K; /* 32000 bytes */

__FLASH_segment_start__               = ORIGIN( rom );
__FLASH_segment_end__                 = __FLASH_segment_start__ + LENGTH( rom );

__SRAM_segment_start__      = ORIGIN( ram );
__SRAM_segment_end__        = __SRAM_segment_start__ + LENGTH( ram );

__privileged_data_start__   = __SRAM_segment_start__;
__privileged_data_end__     = __SRAM_segment_start__ + __privileged_data_region_size__;

__privileged_functions_start__        = __FLASH_segment_start__;
__privileged_functions_end__          = __FLASH_segment_start__ + __privileged_functions_region_size__;


SECTIONS
{
    .privileged_functions :
	{
		_text = .;
		KEEP (*(SORT_NONE(.init)))
        . = ALIGN(4);
        *(privileged_functions)
        . = ALIGN(4);
        FILL(0xDEAD);
        /* Ensure that un-privileged code is placed after the region reserved                                                                                                                                         4          * for privileged kernel code. */
        /* Note that dot (.) actually refers to the byte offset from the start
            * of the current section (.privileged_functions in this case). As a
            * result, setting dot (.) to a value sets the size of the section. */
        . = __privileged_functions_region_size__;
	} >rom AT>rom
	
	.text :
	{
        . = ALIGN(4);
        __syscalls_flash_start__ = .;
        *(freertos_system_calls)
        __syscalls_flash_end__ = .;
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
	} >rom AT>rom
	
	.fini :
	{
		KEEP (*(SORT_NONE(.fini)))
		_etext = .;
	} >rom AT>rom

	.rodata.align ALIGN(4):
	{
		_rodata = .;
	} >rom AT>rom

	.rodata.start :
	{
		_rodata_lma = LOADADDR(.rodata.start);
	} >rom AT>rom

	.rodata :
	{
		*(.rdata)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.r.*)

		. = ALIGN(4);
		_erodata = .;
	} >rom AT>rom

    .privileged_data :
    {
        . = ALIGN(4);
        _sdata = .;        /* Create a global symbol at data start. */
        *(privileged_data)
        . = ALIGN(4);
        FILL(0xDEAD);
        /* Ensure that un-privileged data is placed after the region reserved
            * for privileged kernel data. */
        /* Note that dot (.) actually refers to the byte offset from the start
            * * of the current section (.privileged_data in this case). As a result,
            * * setting dot (.) to a value sets the size of the section. */
        . = __privileged_data_region_size__;
    } >ram AT>rom

	.data.align :
	{
		. = ALIGN(4);
		_data = .;
	} >ram AT>rom

	.data.start :
	{
		_data_lma = LOADADDR(.data.start);
	} >ram AT>rom

	.data :
	{
		*(.data .data.*)
		*(.gnu.linkonce.d.*)
		. = ALIGN(8);
		PROVIDE( __global_pointer$ = . + 0x800 );
		*(.sdata .sdata.*)
		*(.sdata2 .sdata2.*)
		*(.gnu.linkonce.s.*)
		. = ALIGN(8);
		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)

		. = ALIGN(4);
		_edata = .;
	} >ram AT>rom

	.bss.align :
	{
		. = ALIGN(4);
		_bss = .;
	} >ram AT>rom

	.bss.start :
	{
		_bss_lma = LOADADDR(.bss.start);
	} >ram AT>rom

	.bss :
	{
		*(.sbss*)
		*(.gnu.linkonce.sb.*)
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)

		. = ALIGN(4);
		_ebss = .;
	} >ram AT>rom

	. = ALIGN(8);
	_end = .;

	.stack :
	{
		. = ALIGN(16);
		. += __stack_size;
		_stack_top = .;
	} >ram AT>ram
}
