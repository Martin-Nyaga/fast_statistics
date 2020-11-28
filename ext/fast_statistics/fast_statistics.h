#include<stdio.h>
#include<float.h>
#include<ruby.h>

#ifdef HAVE_XMMINTRIN_H
#include<xmmintrin.h>
#endif

VALUE descriptive_statistics(VALUE self, VALUE arrays);

#ifdef HAVE_XMMINTRIN_H
VALUE descriptive_statistics_simd_2x64(VALUE self, VALUE arrays);
#endif
