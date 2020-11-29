#include<stdio.h>
#include<float.h>
#include<ruby.h>

#ifdef HAVE_XMMINTRIN_H
#include<xmmintrin.h>
#endif

#ifdef HAVE_IMMINTRIN_H
#include<immintrin.h>
#endif

VALUE descriptive_statistics(VALUE self, VALUE arrays);

#ifdef HAVE_XMMINTRIN_H
VALUE descriptive_statistics_packed128_float32(VALUE self, VALUE arrays);
VALUE descriptive_statistics_packed128_float64(VALUE self, VALUE arrays);
#endif

#ifdef HAVE_XMMINTRIN_H
VALUE descriptive_statistics_packed256_float32(VALUE self, VALUE arrays);
VALUE descriptive_statistics_packed256_float64(VALUE self, VALUE arrays);
#endif
