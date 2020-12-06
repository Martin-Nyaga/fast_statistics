#include "fast_statistics.hpp"

static VALUE mFastStatistics;

#ifdef HAVE_XMMINTRIN_H
extern "C" VALUE
descriptive_statistics_packed_float64(VALUE self, VALUE arrays)
{
  array_2d::DFloat<double, array_2d::Packed64>* array =
    new array_2d::DFloat<double, array_2d::Packed64>(arrays);
  array_2d::Stats* stats = array->descriptive_statistics_simd();

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_median = ID2SYM(rb_intern("median"));
  VALUE s_q1 = ID2SYM(rb_intern("q1"));
  VALUE s_q3 = ID2SYM(rb_intern("q3"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  for (int i = 0; i < array->cols; i++) {
    VALUE h_result = rb_hash_new();
    array_2d::Stats var_stats = stats[i];

    rb_hash_aset(h_result, s_min, DBL2NUM(var_stats.min));
    rb_hash_aset(h_result, s_max, DBL2NUM(var_stats.max));
    rb_hash_aset(h_result, s_mean, DBL2NUM(var_stats.mean));
    rb_hash_aset(h_result, s_median, DBL2NUM(var_stats.median));
    rb_hash_aset(h_result, s_q1, DBL2NUM(var_stats.q1));
    rb_hash_aset(h_result, s_q3, DBL2NUM(var_stats.q3));
    rb_hash_aset(h_result, s_standard_deviation, DBL2NUM(var_stats.standard_deviation));

    rb_ary_push(a_results, h_result);
  }

  delete array;
  return a_results;
}

extern "C" VALUE
descriptive_statistics_packed_float32(VALUE self, VALUE arrays)
{
  array_2d::DFloat<float, array_2d::Packed32>* array =
    new array_2d::DFloat<float, array_2d::Packed32>(arrays);
  array_2d::Stats* stats = array->descriptive_statistics_simd();

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_median = ID2SYM(rb_intern("median"));
  VALUE s_q1 = ID2SYM(rb_intern("q1"));
  VALUE s_q3 = ID2SYM(rb_intern("q3"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  for (int i = 0; i < array->cols; i++) {
    VALUE h_result = rb_hash_new();
    array_2d::Stats var_stats = stats[i];

    rb_hash_aset(h_result, s_min, DBL2NUM(var_stats.min));
    rb_hash_aset(h_result, s_max, DBL2NUM(var_stats.max));
    rb_hash_aset(h_result, s_mean, DBL2NUM(var_stats.mean));
    rb_hash_aset(h_result, s_median, DBL2NUM(var_stats.median));
    rb_hash_aset(h_result, s_q1, DBL2NUM(var_stats.q1));
    rb_hash_aset(h_result, s_q3, DBL2NUM(var_stats.q3));
    rb_hash_aset(h_result, s_standard_deviation, DBL2NUM(var_stats.standard_deviation));

    rb_ary_push(a_results, h_result);
  }

  delete array;
  return a_results;
}

extern "C" VALUE
simd_enabled(VALUE self)
{
  return Qtrue;
}
#endif

extern "C" VALUE
descriptive_statistics(VALUE self, VALUE arrays)
{
  array_2d::DFloat<double, array_2d::Packed64>* array =
    new array_2d::DFloat<double, array_2d::Packed64>(arrays);
  array_2d::Stats* stats = array->descriptive_statistics();

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_median = ID2SYM(rb_intern("median"));
  VALUE s_q1 = ID2SYM(rb_intern("q1"));
  VALUE s_q3 = ID2SYM(rb_intern("q3"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  for (int i = 0; i < array->cols; i++) {
    VALUE h_result = rb_hash_new();
    array_2d::Stats var_stats = stats[i];

    rb_hash_aset(h_result, s_min, DBL2NUM(var_stats.min));
    rb_hash_aset(h_result, s_max, DBL2NUM(var_stats.max));
    rb_hash_aset(h_result, s_mean, DBL2NUM(var_stats.mean));
    rb_hash_aset(h_result, s_median, DBL2NUM(var_stats.median));
    rb_hash_aset(h_result, s_q1, DBL2NUM(var_stats.q1));
    rb_hash_aset(h_result, s_q3, DBL2NUM(var_stats.q3));
    rb_hash_aset(h_result, s_standard_deviation, DBL2NUM(var_stats.standard_deviation));

    rb_ary_push(a_results, h_result);
  }

  delete array;
  return a_results;
}

extern "C" VALUE
simd_disabled(VALUE self)
{
  return Qfalse;
}

extern "C" void
Init_fast_statistics(void)
{
  mFastStatistics = rb_define_module("FastStatistics");

  // clang-format off
#ifdef HAVE_XMMINTRIN_H
  // Provide double precision packed as default
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics",
      RUBY_METHOD_FUNC(descriptive_statistics_packed_float64), 1);
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics_packed_float32",
      RUBY_METHOD_FUNC(descriptive_statistics_packed_float32), 1);
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics_packed_float64",
      RUBY_METHOD_FUNC(descriptive_statistics_packed_float64), 1);
  rb_define_singleton_method( mFastStatistics, "descriptive_statistics_unpacked",
      RUBY_METHOD_FUNC(descriptive_statistics), 1);
  rb_define_singleton_method(
      mFastStatistics, "simd_enabled?",
      RUBY_METHOD_FUNC(simd_enabled), 0);
#else
  // No SIMD support:
  // Provide all "packed" and "unpacked" versions as the same algorithm
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics",
      RUBY_METHOD_FUNC(descriptive_statistics), 1);
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics_packed_float32",
      RUBY_METHOD_FUNC(descriptive_statistics), 1);
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics_packed_float64",
      RUBY_METHOD_FUNC(descriptive_statistics), 1);
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics_unpacked",
      RUBY_METHOD_FUNC(descriptive_statistics), 1);
  rb_define_singleton_method(
      mFastStatistics, "simd_enabled?",
      RUBY_METHOD_FUNC(simd_disabled), 0);
#endif
  // clang-format on
}
