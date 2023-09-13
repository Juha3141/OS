#ifndef _STRINGS_H_
#define _STRINGS_H_

#include <stdarg.h>

int memcmp(const void *ptr1, const void *ptr2, unsigned int num);
void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *dest, int c, unsigned int n);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *destination, const char *source);
char *strncpy(char *destination, const char *source, unsigned int num);
char *itoa(int num, char *str, int base);
int atoi(const char *str);
char *ltoa(long num, char *str, int base);
long atol(const char *str);
int strncmp(const char *s1, const char *s2, unsigned int n);
unsigned int strlen(const char *str);

void vsprintf(char *Destination , const char *Format , va_list ap);
void sprintf(char *Destination , const char *Format , ...);

#endif