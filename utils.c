/*
 * utils.c
 */
#include <ctype.h>
#include <arpa/inet.h>

int ctype_digit(const char *str)
{
  while (str && *str && isdigit(*str)) str++;
  return (str && *str == '\0')? 1 : 0;
}

int seems_ip4(const char *str)
{
  while (str && *str && (isdigit(*str) || *str == '.')) str++;
  return (str && *str == '\0')? 1 : 0;
}

int seems_ip6(const char *str)
{
  while (str && *str && (isxdigit(*str) || *str == ':')) str++;
  return (str && *str == '\0')? 1 : 0;
}




/* returns bool */
int is_valid_ip4(int family, const char *str)
{
  struct sockaddr_storage addr; /* using sockaddr_storage because it's large enough and we really don't need the converted value */
  return inet_pton(family, str, (void *) &addr) == 1? 1 : 0;
}
