#include "fast_statistics.h"
#include "array_2d.h"

static VALUE rb_m_fast_statistics;

#ifdef HAVE_XMMINTRIN_H
VALUE
simd_enabled(VALUE self) { return Qtrue; }
#else
VALUE
simd_enabled(VALUE self) { return Qfalse; }
#endif

extern "C" void Init_fast_statistics(void) {
  rb_m_fast_statistics = rb_define_module("FastStatistics");
  rb_define_singleton_method(
      rb_m_fast_statistics, "simd_enabled?", RUBY_METHOD_FUNC(simd_enabled), 0);

  array_2d::setup(rb_m_fast_statistics);
}
