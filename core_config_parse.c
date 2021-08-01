/*
 * core_config_parse.c
 */
#include <stdio.h>
#ifdef USE_YAML
#include <yaml.h>
#else
#error "You need to set USE_YAML in CFLAGS. Or at least give alternative."
#endif



int main(int argc, char *argv[])
{
  yaml_parser_t parser;
  FILE *fp = NULL;

  /* Initialize Parser */
  if (!yaml_parser_initialize(&parser)) {
    fprintf(stderr, "Failed to initialize YAML parser!\n");
    goto cleanup;
  }

  fp = fopen("config/core.yaml", "r");

  cleanup:
  if (fp) fclose(fp);
  yaml_parser_delete(&parser);

  return 0;
}
