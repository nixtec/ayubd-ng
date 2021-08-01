// The hello program
#include "prog_lua.h"

prog_out_t *prog_out_new(size_t len)
{
  prog_out_t *po = malloc(sizeof(prog_out_t));
  if (!po) return NULL;
  po->str = calloc(1, len);
  po->len = 0;
  po->alen = len;
  return po;
}

void prog_out_free(prog_out_t *po)
{
  free(po->str);
  free(po);
}

prog_t *prog_init(const char *file)
{
  int i = 0;
  prog_t *prog = calloc(1, sizeof(*prog));;
  prog->L = luaL_newstate();
  luaL_openlibs(prog->L);
  if (luaL_loadfile(prog->L, file) == 0) {
    fprintf(stderr, "%s loaded successfully\n", file);
  } else {
    fprintf(stderr, "%s NOT loaded\n", file);
  }
  lua_setglobal(prog->L, "myfunc");

  return prog;
}

static void stackDump (lua_State *L) {
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) {  /* repeat for each level */
    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
	fprintf(stderr, "String: '%s'", lua_tostring(L, i));
	break;

      case LUA_TBOOLEAN:  /* booleans */
	fprintf(stderr, "Boolean: %s", lua_toboolean(L, i) ? "true" : "false");
	break;

      case LUA_TNUMBER:  /* numbers */
	fprintf(stderr, "Number: %g", lua_tonumber(L, i));
	break;

      default:  /* other values */
	fprintf(stderr, "Others: %s", lua_typename(L, t));
	break;

    }
    fprintf(stderr, "  ");  /* put a separator */
  }
  fprintf(stderr, "\n");  /* end the listing */
}

void prog_exec(prog_t *prog, prog_out_t *po)
{
  int i;
  int result;
  /*
  double a = 19;
  double b = 2;
  */
  double c = 0.0;
  /*
  lua_getglobal(prog->L, "mysum");
  lua_pushnumber(prog->L, a);
  lua_pushnumber(prog->L, b);
  */
  /* Ask Lua to run our little script */
  lua_getglobal(prog->L, "myfunc");

  if (!lua_isfunction(prog->L, -1)) {
    fprintf(stderr, "Not a valid function");
    return;
  }
  result = lua_pcall(prog->L, 0, LUA_MULTRET, 0); // returns 1 value, takes 0 arguments
  if (result) {
    fprintf(stderr, "Failed to run script: %s\n", lua_tostring(prog->L, -1));
    return;
  }
  stackDump(prog->L);
  c = lua_tonumber(prog->L, -1);
  po->len = sprintf(po->str, "ret = %lf\n", c);
  lua_pop(prog->L, -1);
  fprintf(stderr, "ret = %lf\n", c);
}

void prog_done(prog_t *prog)
{
  lua_close(prog->L);
}
