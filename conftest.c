#line 5153 "configure"
#include "confdefs.h"
/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char usleep (); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
#ifdef __cplusplus
extern "C"
#endif
/* We use char because int might match the return type of a gcc2
   builtin and then its argument prototype would still apply.  */
char usleep ();
char (*f) ();

#ifdef F77_DUMMY_MAIN
#  ifdef __cplusplus
     extern "C"
#  endif
   int F77_DUMMY_MAIN() { return 1; }
#endif
int
main ()
{
/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_usleep) || defined (__stub___usleep)
choke me
#else
f = usleep;
#endif

  ;
  return 0;
}
