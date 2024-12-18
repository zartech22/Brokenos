#pragma once
#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
	extern "C" {
#endif

void* memcpy(char *, const char *, unsigned int);
void* memset(void*, uint8_t, size_t);
int strlen(const char*);
int strcmp(const char*, const char*);
void itoa(char*, unsigned long int, unsigned int);
void strcpy(char*, const char*);

void checkBounds(void *ptr, unsigned int bytes);


#ifdef __cplusplus
	}
#endif

class String;
inline bool operator==(const String &str1, const String &str2);

String* stoa(unsigned long int value, unsigned int base);
