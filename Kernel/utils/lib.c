#include "lib.h"
#include "String.h"

#include "memory/kmalloc.h"
#include "video/Screen.h"

void checkBounds(void *ptr, unsigned int bytes)
{
    auto *header = reinterpret_cast<struct kmalloc_header *>(static_cast<char *>(ptr) - sizeof(struct kmalloc_header));

    if(bytes > header->size)
        int x = 3 / 0;
    else if(!header->used)
        int x = 3 / 0;
}

void* memcpy(char *dst, const char *src, unsigned int n)
{
	char *p = dst;
	while(n--)
		*dst++ = *src++;
	return p;
}

void* memset(void *src, const uint8_t val, size_t size)
{
	auto *p = static_cast<uint8_t*>(src);

	while(size--)
		*p++ = val;

	return src;
}

int strlen(const char *s)
{
	int i = 0;

	while (*s++) i++;

	return i;
}

int strcmp(const char *s1, const char *s2)
{
	int size = (strlen(s1) < strlen(s2)) ? strlen(s1) : strlen(s2);

	while(*s1++ == *s2++ && --size);

	if(size != 0)
		return (*--s1 < *--s2) ? -1 : 1;
	else
		return 0;
}

void itoa(char *buf, unsigned long int n, const unsigned int base)
{
	unsigned long int tmp;
	int i = 0;

	do {
		tmp = n % base;
		buf[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'A' - 10);
	} while (n /= base);

	buf[i--] = 0;

	for (int j = 0; j < i; j++, i--) {
		tmp = buf[j];
		buf[j] = buf[i];
		buf[i] = tmp;
	}
}

String* stoa(const unsigned long int value, const unsigned int base)
{
	const auto buffer = new char[512];

	itoa(buffer, value, base);

	return new String(buffer);
}

void strcpy(char* dest, const char* src)
{
    unsigned int i = 0;

    while(src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';
}

inline bool operator== (const String &str1, const String &str2)
{
    return (strcmp(str1.c_str(), str2.c_str()) == 0);
}
