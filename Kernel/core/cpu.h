#ifndef CPU_H
#define CPU_H

#include <utils/String.h>

class CPU
{
public:
    static String getVendor()
    {
        u32 *vendor = (u32*)kmalloc(13);

        asm volatile("xor %%eax, %%eax;"
                    "cpuid;"
                    : "=b" (*vendor), "=c" (*(vendor + 2)), "=d" (*(vendor + 1)) :: "%eax");

        vendor[13] = '\0';

        return String((char*)vendor);
    }

    static String getBrand()
    {
        u32 *brand = (u32*)kmalloc(49);

        asm volatile("cpuid;" : "=a" (*brand), "=b" (*(brand + 1)), "=c" (*(brand + 2)), "=d" (*(brand + 3)) : "a" (0x80000002));
        asm volatile("cpuid;" : "=a" (*(brand + 4)), "=b" (*(brand + 5)), "=c" (*(brand + 6)), "=d" (*(brand + 7)) : "a" (0x80000003));
        asm volatile("cpuid;" : "=a" (*(brand + 8)), "=b" (*(brand + 9)), "=c" (*(brand + 10)), "=d" (*(brand + 11)) : "a" (0x80000004));

        brand[49] = '\0';

        return String((char*)brand);
    }
};

#endif // CPU_H
