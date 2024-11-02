#pragma once

#include <utils/String.h>

class CPU
{
public:
    static String getVendor()
    {
        auto vendor = static_cast<u32 *>(kmalloc(13));

        asm ("xor %%eax, %%eax;"
                    "cpuid;"
                    : "=b" (*vendor), "=c" (*(vendor + 2)), "=d" (*(vendor + 1)) :: "%eax");

        vendor[12] = '\0';

        String res(reinterpret_cast<char *>(vendor));

        kfree(vendor);

        return res;
    }

    static String getBrand()
    {
        auto brand = static_cast<u32 *>(kmalloc(49));

        asm ("cpuid;" : "=a" (*brand), "=b" (*(brand + 1)), "=c" (*(brand + 2)), "=d" (*(brand + 3)) : "a" (0x80000002));
        asm ("cpuid;" : "=a" (*(brand + 4)), "=b" (*(brand + 5)), "=c" (*(brand + 6)), "=d" (*(brand + 7)) : "a" (0x80000003));
        asm ("cpuid;" : "=a" (*(brand + 8)), "=b" (*(brand + 9)), "=c" (*(brand + 10)), "=d" (*(brand + 11)) : "a" (0x80000004));

        brand[48] = '\0';

        String res(reinterpret_cast<char *>(brand));

        kfree(brand);

        return res;
    }
};
