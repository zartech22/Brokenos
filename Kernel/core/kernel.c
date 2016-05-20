#include <utils/types.h>
#include <memory/gdt.h>
#include <video/Screen.h>
#include <core/io.h>
#include <memory/idt.h>
#include <memory/mm.h>
#include <core/process.h>
#include <utils/lib.h>
#include <memory/kmalloc.h>
#include <pci/pci.h>
#include <disk/ide.h>
#include <utils/elf.h>

#include <utils/String.h>
#include <utils/Vector.h>

#include <core/cpu.h>

void init_pic();
int main(mb_partial_info *multiboot_info);

#ifdef __cplusplus
extern "C" {
#endif
	void kmain(struct mb_partial_info*);
#ifdef __cplusplus
	}
#endif

void kmain(struct mb_partial_info *memInfo)
{
	cli;

    main(memInfo);
}

int main(struct mb_partial_info *mbinfo)
{
    init_gdt();
    // Update SS and ESP
    asm("movw $0x18, %%ax	\n \
         movw %%ax, %%ss		\n \
         movl %0, %%esp" :: "i"(KERN_STACK));

    init_idt();
    init_mm(mbinfo->high_mem);

    Screen::initScreen((struct VbeModeInfo*)mbinfo->vbe_mode_info);

    Screen &s = Screen::getScreen();

    s.printInfo("Kernel charge en memoire !");
    s.okMsg();
    s.printInfo("kernel : chargement idt...");
    s.okMsg();
    s.printInfo("kernel : initialisation de la memoire...");
    s.println("kernel : paging actif !");

    s.printDebug("Info memoire : %uk (lower) %uk (upper)", mbinfo->low_mem, mbinfo->high_mem);

    s.printInfo("kernel : chargement nouvelle gdt...");
    s.okMsg();
    s.printInfo("\t->Reaffectation du registre SS et ESP...");

    struct VbeModeInfo *info = (struct VbeModeInfo*)mbinfo->vbe_mode_info;

    s.printDebug("VBE BitsPerPixel %u, %u, %u,  %p", info->XResolution, info->YResolution, info->BitsPerPixel, info->PhysBasePtr);


    if(mbinfo->flags & 0x800)
        s.printDebug("C'est good ! %s", mbinfo->boot_loader_name);
    else
        s.printDebug("La merde...");


    s.printInfo("kernel : configuration du PIC...");
    init_pic();
    s.okMsg();

    s.printInfo("kernel : init tss");
    asm("	movw $0x38, %ax \n \
            ltr %ax");
    s.okMsg();

    s.printInfo("kernel : reactivation des interruptions...");
    s.okMsg();

    s.printInfo("Initialisation de l'horloge...");
	outb(0x43, 0x34);
	outb(0x40, (0x1234DE / 50) & 0x00FF);
    outb(0x40, (0x1234DE / 50) >> 8);
    s.okMsg();


    s.printInfo("Kernel pret a l'action !");

    s.hide_cursor();

	//Init kernel thread
	current = &p_list[0];
	current->pid = 0;
	current->state = 1;
	current->regs.cr3 = (u32) pd0;

    s.println("Vendor : %s", CPU::getVendor().c_str());
    s.println("Brand : %s", CPU::getBrand().c_str());

    s.printDebug("PCI list :");
	pciGetVendors();

    IdeDrive &d = ctrl.getDrive(PrimaryBus, Master);

    d.displayPartitions();

    s.printInfo("kernel : tasks created");

    s.printInfo("kernel : scheduler enabled");

    s.printError("Address : %p", info->PhysBasePtr);

    sti;

	for(;;)
		asm("hlt");
}
