/*
 * defines.h
 * MACRO definitions
 */
#define TOSTR(s) #s
#define LEN(S) (sizeof(s)-1)
#define LENSTR(s) TOSTR(LEN(s)-1)


#define NUL     '\0'
#define CRLF    "\r\n"
#define COLON   ":"
#define SPACE   " "
#define RS      "?"
#define FS      "&"

#define ERROR 1
#define __K__ 0

#define CHECKDIGIT(x) (((x) >= '0' && (x) <= '9') ? __K__ : ERROR)
#define CHECKXCHAR(x) ((((x) >= 'A' && (x) <= 'F') || ((x) >= 'a' && (x) <= 'f')) ? __K__ : ERROR)
/* 'x' must not have side effects */
#define HEXCHARVAL(x) (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A' +10) : (((x) >= 'a' && (x) <= 'f') ? ((x) - 'a' + 10) : ((x)))))

#define TOUPPER(c) (((c)>='a' && (c) <='z')?(c)-(' '):(c))
#define TOLOWER(c) (((c)>='A' && (c) <='Z')?(c)+(' '):(c))

#define STRTOUPPER(str,len) do {				\
  register int i;						\
  for (i = 0; i < len; ++i) {					\
    *((str)+i) = TOUPPER(*((str)+i));				\
  }								\
} while (0)

#define STRTOLOWER(str,len) do {				\
  register int i;						\
  for (i = 0; i < len; ++i) {					\
    *((str)+i) = TOLOWER(*((str)+i));				\
  }								\
} while (0)

