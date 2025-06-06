MEMORY {
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 128K
	RAM (rw)   : ORIGIN = 0x20000000, LENGTH = 32K
	IRAM (rwx) : ORIGIN = 0x20100000, LENGTH = 16K
}

ENTRY(__isr_reset)

SECTIONS {
	.text : {
		KEEP(*(.isr_vector))
		
		*(.text*)
		*(.rodata*)

		*(.font*)

		*(.ARM*)

		. = ALIGN(4);
		__text_end__ = .;
	} > FLASH

	.data : AT (__text_end__) {
		. = ALIGN(4);
		__data_start__ = .;

		*(.data*)

		. = ALIGN(4);
		__data_end__ = .;
	} > RAM

	.bss : {
		. = ALIGN(4);
		__bss_start__ = .;

		*(.bss*)

		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM

	.heap (NOLOAD): {
		. = ALIGN(8);
		__heap_start__ = .;

		*(.heap*)

		. = ALIGN(8);
		__heap_end__ = .;
	} > RAM

	.stack (NOLOAD): {
		. = ALIGN(8);
		__stack_limit__ = .;

		*(.stack*)

		. = ALIGN(8);
		__stack_top__ = .;
	} > RAM

	__stack_top__ = ORIGIN(RAM) + LENGTH(RAM);
	__stack_limit__ = __stack_top__ - SIZEOF(.stack);
	ASSERT(__stack_limit__ > __heap_end__, "region ram overflowed with stack")
}
