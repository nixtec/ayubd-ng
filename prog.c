#ifdef USE_PROG

#ifdef USE_PH7

#define PROG_DEMO "<?php "\
                 "echo 'Hello World'.PHP_EOL;"\
                 "echo 'Current system time is: '.date('Y-m-d H:i:s').PHP_EOL;"\
                 "echo 'and you are running '.php_uname();"\
                 "?>"
#endif /* !USE_PH7 */

#ifdef USE_UNQLITE

#define PROG_DEMO  "print 'Hello World', JX9_EOL;"
#endif /* !USE_UNQLITE */

/* Make sure you have the latest release of the PH7 engine
 * from:
 *  http://ph7.symisc.net/downloads.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"

prog_in_t *prog_in_new(size_t len)
{
  prog_in_t *pi = malloc(sizeof(prog_in_t));
  if (!pi) return NULL;
  pi->str = NULL; // it will just point, not memory needed
  pi->len = 0;
  return pi;
}

void prog_in_free(prog_in_t *pi)
{
  free(pi);
}


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


/* 
 * Display an error message and exit.
 */
static void Fatal(const char *zMsg)
{
	puts(zMsg);
	/* Shutdown the library */
#ifdef USE_PH7
	ph7_lib_shutdown();
#endif
#ifdef USE_UNQLITE
	unqlite_lib_shutdown();
#endif
	/* Exit immediately */
	exit(0);
}

static void Warn(const char *zMsg)
{
	puts(zMsg);
}
/*
 * VM output consumer callback.
 * Each time the virtual machine generates some outputs, the following
 * function gets called by the underlying virtual machine  to consume
 * the generated output.
 * All this function does is redirecting the VM output to STDOUT.
 * This function is registered later via a call to ph7_vm_config()
 * with a configuration verb set to: PH7_VM_CONFIG_OUTPUT.
 */
static int Output_Consumer(const void *pOutput, unsigned int nOutputLen, void *pUserData /* Unused */)
{
  prog_out_t *po = pUserData;
  size_t po_pos = 0;
  size_t po_apos = 0;
  size_t po_remaining = 0;
  size_t outlen = 0;
  //fprintf(stderr, "Consuming Output of length: %u\n", nOutputLen);
  if (po) {
    po_pos = po->len;
    po_apos = po->alen;
    po_remaining = po_apos - po_pos;


    //po->len = nOutputLen >= po->alen - 1 ? po->alen-1 : nOutputLen;
    outlen = nOutputLen > po_remaining ? po_remaining : nOutputLen;
    if (outlen > 0) {
      memcpy(po->str+po_pos, pOutput, outlen);
      po->len = po_pos + outlen;
    }
    //*(char *)(po->str+po->len) = '\0'; fprintf(stderr, "[length: %lu]: %s\n", po->len, po->str);
  }
	/* 
	 * Note that it's preferable to use the write() system call to display the output
	 * rather than using the libc printf() which everybody now is extremely slow.
	 */
#if 0
	printf("%.*s", 
		nOutputLen, 
		(const char *)pOutput /* Not null terminated */
		);
#endif
	/* All done, VM output was redirected to STDOUT */
#ifdef USE_PH7
	return PH7_OK;
#endif
#ifdef USE_UNQLITE
	return UNQLITE_OK;
#endif
}

/* 
 * Main program: Compile and execute the PHP program defined above.
 */
prog_t *prog_init(const char *file)
{
  prog_t *prog = calloc(1, sizeof(*prog));;

  int rc;
#ifdef USE_PH7
  rc = ph7_init(&prog->pEngine);
  if ( rc != PH7_OK ){
#endif
#ifdef USE_UNQLITE
  rc = unqlite_open(&prog->pEngine, PROG_DB, UNQLITE_OPEN_READONLY | UNQLITE_OPEN_MMAP);
  if ( rc != UNQLITE_OK ){
#endif
    /*
     * If the supplied memory subsystem is so sick that we are unable
     * to allocate a tiny chunk of memory, there is no much we can do here.
     */
    Fatal("Error while initializing engine/database instance.");
  }


  /* AYUB: FIXME: Define one time tasks here */
  if (file) {
    fprintf(stderr, "Compiling file %s\n", file);
#ifdef USE_PH7
    rc = ph7_compile_file(
	prog->pEngine,
	file, /* test program */
	&prog->pVm,     /* OUT: Compiled VM */
	0         /* IN: Compile flags */
	);
#endif
#ifdef USE_UNQLITE
    rc = unqlite_compile_file(
	prog->pEngine,
	file, /* test program */
	&prog->pVm     /* OUT: Compiled VM */
	);
#endif
  } else {
#ifdef USE_PH7
    rc = ph7_compile_v2(
	prog->pEngine,  /* engine */
	PROG_DEMO, /* test program */
	-1        /* Compute input length automatically*/, 
	&prog->pVm,     /* OUT: Compiled VM */
	0         /* IN: Compile flags */
	);
#endif
#ifdef USE_UNQLITE
    rc = unqlite_compile(
	prog->pEngine,  /* engine */
	PROG_DEMO, /* test program */
	-1        /* Compute input length automatically*/, 
	&prog->pVm     /* OUT: Compiled VM */
	);
#endif
  }


#ifdef USE_PH7
  if( rc != PH7_OK ){
    if( rc == PH7_COMPILE_ERR ){
#endif
#ifdef USE_UNQLITE
  if( rc != UNQLITE_OK ){
    if( rc == UNQLITE_COMPILE_ERR ){
#endif
      const char *zErrLog;
      int nLen;
      /* Extract error log */
#ifdef USE_PH7
      ph7_config(prog->pEngine, PH7_CONFIG_ERR_LOG, 
#endif
#ifdef USE_UNQLITE
      unqlite_config(prog->pEngine, UNQLITE_CONFIG_JX9_ERR_LOG, 
#endif
	  &zErrLog, 
	  &nLen
	  );
      if( nLen > 0 ){
	/* zErrLog is null terminated */
	puts(zErrLog);
      }
    }
    /* Exit */
    Fatal("Compile error");
  }




  /*
   * And finally, execute our program. Note that your output (STDOUT in our case)
   * should display the result.
   */
  return prog;
}

void prog_exec(prog_t *prog, prog_in_t *pi, prog_out_t *po)
{
  int rc;

#ifdef USE_PH7
  /* parse header and populate $_* values */
  rc = ph7_vm_config(prog->pVm, 
      PH7_VM_CONFIG_HTTP_REQUEST, 
      pi->str,
      pi->len
      );
  if( rc != PH7_OK ){
    Warn("Error while parsing HTTP REQUEST.");
  }
#endif


  /*
   * Now we have our script compiled, it's time to configure our VM.
   * We will install the VM output consumer callback defined above
   * so that we can consume the VM output and redirect it to STDOUT.
   */
#ifdef USE_PH7
  rc = ph7_vm_config(prog->pVm, 
      PH7_VM_CONFIG_OUTPUT, 
#endif
#ifdef USE_UNQLITE
  rc = unqlite_vm_config(prog->pVm, 
      UNQLITE_VM_CONFIG_OUTPUT, 
#endif
      Output_Consumer,    /* Output Consumer callback */
      po                /* Callback private data */
      );
#ifdef USE_PH7
  if( rc != PH7_OK ){
#endif
#ifdef USE_UNQLITE
  if( rc != UNQLITE_OK ){
#endif
    Fatal("Error while installing the VM output consumer callback");
  }

#ifdef USE_PH7
  ph7_vm_exec(prog->pVm, 0);
  if (ph7_vm_reset(prog->pVm) != PH7_OK) {
#endif
#ifdef USE_UNQLITE
  unqlite_vm_exec(prog->pVm);
  if (unqlite_vm_reset(prog->pVm) != UNQLITE_OK) {
#endif
    Warn("Error resetting VM");
  }
}

void prog_done(prog_t *prog)
{
  /*
   * All done, cleanup the mess left behind.
   */
#ifdef USE_PH7
  ph7_vm_release(prog->pVm);
  ph7_release(prog->pEngine);
#endif

#ifdef USE_UNQLITE
  unqlite_vm_release(prog->pVm);
  unqlite_close(prog->pEngine);
#endif

  free(prog);
}

#if 0

int main(int argc, char *argv[])
{
  prog_t *prog = NULL;

  if (argc > 1) {
    prog = prog_init(argv[1]);
    if (prog) {
      prog_exec(prog);
      prog_done(prog);
    }
  }

  return 0;
}

#endif

#endif /* !USE_PROG */
