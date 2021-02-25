#ifndef FAST_STATISTICS_H
#define FAST_STATISTICS_H

#include <ruby.h>
#ifdef HAVE_XMMINTRIN_H
#include <xmmintrin.h>
#endif

#include "array_2d.h"
using namespace array_2d;

#define rb_sym(str) ID2SYM(rb_intern(str))

#endif
