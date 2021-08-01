/*
 * utils.h
 */
#ifndef __UTILS_H__
#define __UTILS_H__

int ctype_digit(const char *str);
int seems_ip4(const char *str);
int seems_ip6(const char *str);
int is_valid_ip(int family, const char *str);

#endif
