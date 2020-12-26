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

#include <interrupt/interrupt.h>

#include <utils/linkedlist.h>

#include <disk/FileSystems/Ext2/Ext2FS.h>

void init_pic();
int main(struct mb_partial_info *multiboot_info);

extern "C" void kmain(struct mb_partial_info*);

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

    struct mmapInfo mmap[1024];
    u32 size = mbinfo->mmap_length;
    int index = 0;

     if(mbinfo->flags & (1 << 6))
     {
         auto *currentMmap = reinterpret_cast<struct mmapInfo*>(mbinfo->mmap_addr);

         while((u32)currentMmap < (mbinfo->mmap_addr + mbinfo->mmap_length))
         {
             mmap[index].addr_low = currentMmap->addr_low;
             mmap[index].addr_high = currentMmap->addr_high;

             mmap[index].length_low = currentMmap->length_low;
             mmap[index].length_high = currentMmap->length_high;

             mmap[index].type = currentMmap->type;

             currentMmap += currentMmap->size + sizeof(currentMmap->size);
             ++index;
         }
     }

    init_idt();
    init_mm(mbinfo->high_mem);

    Screen::initScreen(mbinfo);

    sScreen.printDebug("MMap info ? %b", mbinfo->flags & (1 << 6));

    sScreen.printDebug("Mmap Length : %d", size);

    for(u32 j = 0; j < index; ++j)
    {
        sScreen.printDebug("\t%p - %p ; Length : %x Type : %d", mmap[j].addr_low, (mmap[j].addr_low + mmap[j].length_low), mmap[j].length_low, mmap[j].type);
        if(mmap[j].addr_high != 0 || mmap[j].length_high != 0)
            sScreen.printDebug("Lol...");
    }

    sScreen.printInfo("Kernel charge en memoire !");
    sScreen.okMsg();
    sScreen.printInfo("kernel : chargement idt...");
    sScreen.okMsg();
    sScreen.printInfo("kernel : initialisation de la memoire...");
    sScreen.println("kernel : paging actif !");

    sScreen.printDebug("Info memoire : %uk (lower) %uk (upper)", mbinfo->low_mem, mbinfo->high_mem);

    sScreen.printInfo("kernel : chargement nouvelle gdt...");
    sScreen.okMsg();
    sScreen.printInfo("\t->Reaffectation du registre SS et ESP...");

    auto *info = reinterpret_cast<struct VbeModeInfo*>(mbinfo->vbe_mode_info);

    sScreen.printDebug("VBE BitsPerPixel %u, %u, %u,  %p", info->XResolution, info->YResolution, info->BitsPerPixel, info->PhysBasePtr);

    if(mbinfo->flags & 0x800)
        sScreen.printDebug("Loader : %s", mbinfo->boot_loader_name);

    sScreen.printInfo("kernel : configuration du PIC...");
    init_pic();
    sScreen.okMsg();

    sScreen.printInfo("kernel : init tss");
    asm("	movw $0x38, %ax \n \
            ltr %ax");
    sScreen.okMsg();

    sScreen.printInfo("kernel : reactivation des interruptions...");
    sScreen.okMsg();

    sScreen.printInfo("Initialisation de l'horloge...");
	outb(0x43, 0x34);
	outb(0x40, (0x1234DE / 50) & 0x00FF);
    outb(0x40, (0x1234DE / 50) >> 8);
    sScreen.okMsg();


    sScreen.printInfo("Kernel pret a l'action !");

    sScreen.hide_cursor();

	//Init kernel thread
	current = &p_list[0];
	current->pid = 0;
	current->state = 1;
	current->regs.cr3 = (u32) pd0;

    //n_proc = 0;
    //n_proc++;

    sScreen.println("Vendor : %s", CPU::getVendor().c_str());
    sScreen.println("Brand : %s", CPU::getBrand().c_str());

    sScreen.printDebug("PCI list :");
	pciGetVendors();

	Vector<IdeCtrl*> &ideControllerList = IdeCtrl::getControllerList();
	const u8 controllerCount = ideControllerList.size();

    sScreen.println("Nombre de controlleur IDE : %d", controllerCount);

    if(controllerCount != 0)
    {
        for(int i = 0; i < controllerCount; ++i)
        {
            ideControllerList[i]->displayTree();

            if(ideControllerList[i]->getDrive(PrimaryBus, Master).isConnected())
                ideControllerList[i]->getDrive(PrimaryBus, Master).displayPartitions();
        }
    }



    sScreen.printInfo("kernel : tasks created");
    sScreen.printInfo("kernel : scheduler enabled");

    //s.printError("Address : %p", info->PhysBasePtr);

    //struct file *test = FileSystem::getFsList().at(0)->getFile("/boot/task2");
    //char *content = FileSystem::getFsList().at(0)->readFile("/boot/task1");

    //s.printError("File content : %c %c %c %c", content[0], content[1], content[2], content[3]);

    //Screen::getScreen().printInfo("isElf : %s", isElf(content) ? "True" : "False");


    //s.printDebug("Taille fichier : %s, %u", test->name, test->size);

//    FileSystem *fs = FileSystem::getFsList().at(0);
    sScreen.printDebug("Nb FS : %b", (&(FileSystem::getFsList()) == 0));

//    s.printError("\tStart LBA : %u", fs->getPartition().s_lba);
//    auto f = fs->getFile("foo.txt");
//    s.printError("\tStart LBA : %u", fs->getPartition().s_lba);

//    s.print("\n\n\n\n");
//    char *content = fs->readFile(f);
//    s.printError("\tStart LBA : %u", fs->getPartition().s_lba);

//    if(!content)
//        s.printError("FileError");

//    s.print("Content of %s, size %d : \n", f->name, f->size);
//    for(int i = 0; i < f->size; ++i)
//        s.print("%c", content[i]);
//    s.print("\n");

    load_task("/boot/task1");
    /*load_task("/boot/task2");
    load_task("/boot/task3");*/

    sti;

    createThread((char*)0xBADA55);

	for(;;)
		asm("hlt");
}
