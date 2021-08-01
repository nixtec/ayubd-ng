/*
 * strsep-test.c
 */
#include <stdio.h>
#include <string.h>

typedef struct {
  char *ptr;
  size_t len;
} token_t;

/*
typedef struct {
  int s;
  int l;
} tinfo;
*/
 
int main(int argc, char *argv[])
{
  //char string[] = "A B C";
  char string[4096] = {};
  char *stringp = string;
  const char *delim = ",";
  char *token = NULL;

  int pos = 2;
  int i = 0;
  int len = 0;

  while (fgets(string, sizeof(string), stdin)) {
    stringp = string;

    i = 0;
    while (stringp != NULL) {
      i++;
      token = strsep(&stringp, delim);
      if (i == pos) {
	//puts(token+2);
	len = stringp - token - 1;
	//sscanf(token+2, "%d", &msisdn);
	printf("%s (%d)\n", token, len);
	break;
      }
    }
  }

  return 0;
}
