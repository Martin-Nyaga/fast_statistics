#ifndef FAST_STATISTICS_H
#define FAST_STATISTICS_H

#include <ruby.h>
#ifdef HAVE_XMMINTRIN_H
  #ifdef __x86_64__
     #include <xmmintrin.h>
  #else
    #include "sse2neon.h"
  #endif
#endif

#include "debug.h"

#define rb_sym(str) ID2SYM(rb_intern(str))
#define UNWRAP_DFLOAT(obj, var) TypedData_Get_Struct((obj), void*, &dfloat_wrapper, (var));

#endif
