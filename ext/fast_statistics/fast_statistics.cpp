#include "fast_statistics.h"
#include "array_2d.h"

static VALUE mFastStatistics;

#ifdef HAVE_XMMINTRIN_H
VALUE
simd_enabled(VALUE self)
{
  return Qtrue;
}
#else
VALUE
simd_enabled(VALUE self)
{
  return Qfalse;
}
#endif

extern "C" void
Init_fast_statistics(void)
{
  mFastStatistics = rb_define_module("FastStatistics");
  rb_define_singleton_method(mFastStatistics, "simd_enabled?", RUBY_METHOD_FUNC(simd_enabled), 0);

  array_2d::initialize(mFastStatistics);
}
