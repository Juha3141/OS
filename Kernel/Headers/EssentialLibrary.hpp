#ifndef _ESSENTIAL_LIBRARY_HPP_
#define _ESSENTIAL_LIBRARY_HPP_

#include <stdarg.h>

extern void *memset(void *Destination , unsigned char Value , unsigned long Size);
extern void *memcpy(void *Destination , const void *Source , unsigned long Size);
extern int memcmp(const void *Buffer1 , const void *Buffer2 , unsigned long Size);

extern unsigned long strlen(const char *String);
extern char *strcpy(char *Destination , const char *Origin);
extern char *strncpy(char *Destination , const char *Origin , unsigned long Length);
extern char *strcat(char *Destination , const char *Origin);
extern char *strncat(char *Destination , const char *Origin , unsigned long Length);
extern int strcmp(const char *String1 , const char *String2);
extern int strncmp(const char *String1 , const char *String2 , unsigned long Length);

extern int ToString(char *String , int Value);
extern int ToString(char *String , long Value);
extern int ToString(char *String , long long Value);
extern int ToString(char *String , unsigned long Value);
extern int ToString(char *String , unsigned long long Value);
extern int ToString(char *String , float Value);
extern int ToString(char *String , double Value);
extern int ToString(char *String , long double Value);

extern int ToInteger(const char *String);

extern int vsprintf(char *String , const char *Formt , va_list ap);
extern int sprintf(char *String , const char *Format , ...);

#endif