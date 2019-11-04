#ifndef STDLIB_H_
#define STDLIB_H_
#include <stdint.h>
#define NULL ((void *) 0)

int strlen(char* str);

int strcmp(char* str1, char* str2);

int strncmp(char* str1, char* str2, int n);

int itoa(int num, char* string, int base);

int atoi(char * s);

int atox(char * s);

void * malloc(uint64_t size);

void free(void * address);

unsigned int lineCount(char * input);

char * filterVowels(char * dest, char * src);

char *trim(char *input);

uint64_t replaceChars(char *input, char target, char newChar);

#endif /* STDLIB_H_ */