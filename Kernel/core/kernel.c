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
#include <disk/IDE/IdeCtrl.h>
#include <disk/IDE/IdeDrive.h>
#include <utils/elf.h>
#include <disk/FileSystems/FileSystem.h>

#include <utils/String.h>
#include <utils/Vector.h>

#include <core/cpu.h>




#include <disk/FileSystems/Ext2/Ext2FS.h>

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

    //n_proc = 0;
    //n_proc++;

    s.println("Vendor : %s", CPU::getVendor().c_str());
    s.println("Brand : %s", CPU::getBrand().c_str());

    s.printDebug("PCI list :");
	pciGetVendors();

    IdeDrive &d = ctrl.getDrive(PrimaryBus, Master);

    d.displayPartitions();

    s.printInfo("kernel : tasks created");

    s.printInfo("kernel : scheduler enabled");

    s.printError("Address : %p", info->PhysBasePtr);

    //struct file *test = FileSystem::getFsList().at(0)->getFile("/boot/task2");
    char *content = FileSystem::getFsList().at(0)->readFile("/boot/task1");

    s.printError("File content : %c %c %c %c", content[0], content[1], content[2], content[3]);

    Screen::getScreen().printInfo("isElf : %s", isElf(content) ? "True" : "False");


    //s.printDebug("Taille fichier : %s, %u", test->name, test->size);

    /*FileSystem *fs = FileSystem::getFsList().at(0);

    s.printError("\tStart LBA : %u", fs->getPartition().s_lba);
    auto f = fs->getFile("foo.txt");
    s.printError("\tStart LBA : %u", fs->getPartition().s_lba);

    s.print("\n\n\n\n");
    char *content = fs->readFile(f);
    s.printError("\tStart LBA : %u", fs->getPartition().s_lba);

    if(!content)
        s.printError("FileError");

    s.print("Content of %s, size %d : \n", f->name, f->size);
    for(int i = 0; i < f->size; ++i)
        s.print("%c", content[i]);
    s.print("\n");*/

    load_task("/boot/task1");
    load_task("/boot/task2");
    load_task("/boot/task3");

    sti;

	for(;;)
    {
        //Screen::getScreen().printDebug("Kernel : %d process running", n_proc);
        //for(int i = 0; i < 10000000; i++);
		asm("hlt");
    }
}
