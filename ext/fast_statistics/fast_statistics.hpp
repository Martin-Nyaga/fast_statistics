#include <float.h>
#include <ruby.h>

#include "array_2d.hpp"

extern "C" VALUE
descriptive_statistics(VALUE self, VALUE arrays);

#ifdef HAVE_XMMINTRIN_H
#include <xmmintrin.h>

extern "C" VALUE
descriptive_statistics_packed_float32(VALUE self, VALUE arrays);
extern "C" VALUE
descriptive_statistics_packed_float64(VALUE self, VALUE arrays);
#endif
