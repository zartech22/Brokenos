ENTRY(_start)
OUTPUT(kernel)

SECTIONS
{
	. = 100000;

	.text :
	{
		*(.multiboot)
		*(.text)
	}

	.rodata :
	{
		*(.rodata)
	}

	.data :
	{
		*(.data)
	}

	.bss :
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
}
