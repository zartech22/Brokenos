#ifndef _LIB_
#define _LIB_

#ifdef __cplusplus
	extern "C" {
#endif

void* memcpy(char *, const char *, unsigned int);
void* memset(char*, int, unsigned int);
int strlen(const char*);
int strcmp(const char*, const char*);
void itoa(char*, unsigned long int, int);
void strcpy(char*, const char*);

#ifdef __cplusplus
	}
#endif

class String;
bool operator==(const String &str1, const String &str2);

#endif
