#include "lib.h"

void* memcpy(char *dst, const char *src, unsigned int n)
{
	char *p = dst;
	while(n--)
		*dst++ = *src++;
	return p;
}

void* memset(char *src, int val, unsigned int size)
{
	char *p = src;
	
	while(size--)
		*src++ = val;
	
	return p;
}

int strlen(const char *s)
{
	int i = 0;
	
	while (*s++) i++;
	
	return i;
}

void itoa(char *buf, unsigned long int n, int base)
{
	unsigned long int tmp;
	int i;

	tmp = n;
	i = 0;

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
