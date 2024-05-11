#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	size_t i = 0;
	for ( ;s[i] != '\0'; i ++) {

	}
	return i;
}

char *strcpy(char *dst, const char *src) {
	size_t i;
	for (i = 0; src[i] != '\0'; i ++) {
		dst[i] = src[i];
	}
	dst[i] = '\0';
	return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++) {
		dst[i] = src[i];
	}
	for ( ; i < n; i ++) {
		dst[i] = '\0';
	}

	return dst;
}

char *strcat(char *dst, const char *src) {
	size_t i;
	size_t j;
	for (i = 0; dst[i] != '\0'; i ++) {

	}
	for (j = 0; src[j] != '\0'; ) {
		dst[i ++] = src[j ++];
	}
	dst[i] = '\0';
	return dst;
}

int strcmp(const char *s1, const char *s2) {
	size_t i;
	for (i = 0; s1[i] != '\0' && s2[i] != '\0'; i ++) {
		if (s1[i] == s2[i]) {
			continue;
		} 
		else {
			return s1[i] - s2[i];
		}
	}

	if (s1[i] == '\0' && s2[i] != '\0') {
		return -s2[i];
	} 
	else if (s1[i] != '\0' && s2[i] == '\0') {
		return s1[i];
	} 
	else {
		return 0;
	}
}

int strncmp(const char *s1, const char *s2, size_t n) {
	size_t i;
	for (i = 0; i < n && s1[i] != '\0' && s2[i] != '\0'; i ++) {
		if (s1[i] == s2[i]) {
			continue;
		} 
		else {
			return s1[i] - s2[i];
		} 
	}
	if (i == n) {
		return 0;
	} 
	else if (s1[i] == '\0' && s2[i] != '\0') {
		return -s2[i];
	} 
	else if (s1[i] != '\0' && s2[i] == '\0') {
		return s1[i];
	} 
	else {
		return 0;
	}
}

void *memset(void *s, int c, size_t n) {
  unsigned char *ptr = (unsigned char*)s;
	for ( ; n > 0; n --) {
		*ptr = c;
		ptr ++;
	}
	return s;
}

void *memmove(void *dst, const void *src, size_t n) {
	char *d = dst;
	const char *s = src;
	if (d < s) {
		for ( ; n > 0; n --) {
			*d = *s;
			d ++; s ++;
		}	
	} 
	else {
		const char *lasts = s + (n - 1);
		char *lastd = d + (n - 1);
		for ( ; n > 0; n --) {
			*lastd = *lasts;
			lastd --; lasts --;
		}
	} 
	return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
	char *d = out;
	const char *s = in;
	for ( ; n > 0; n -- ) {
		*d = *s;
		d ++; s ++;
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const unsigned char *str1 = s1;
	const unsigned char *str2 = s2;

	for ( ; n > 0; n --) {
		if (*str1 != *str2) {
			return (*str1 - *str2);
		}
		str1 ++; str2 ++;
	}
	return 0;
}

#endif
