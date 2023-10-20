#include "fast_statistics.h"
#include "array_2d.h"

using namespace array_2d;

static VALUE mFastStatistics;
static VALUE cArray2D;

// Helper
VALUE
build_results_hashes(Stats* stats, int num_variables)
{
  VALUE a_results = rb_ary_new();

  VALUE s_min = rb_sym("min");
  VALUE s_max = rb_sym("max");
  VALUE s_mean = rb_sym("mean");
  VALUE s_median = rb_sym("median");
  VALUE s_q1 = rb_sym("q1");
  VALUE s_q3 = rb_sym("q3");
  VALUE s_standard_deviation = rb_sym("standard_deviation");
  VALUE s_skew_median_pearson = rb_sym("skew_median_pearson");

  for (int i = 0; i < num_variables; i++) {
    VALUE h_result = rb_hash_new();
    Stats var_stats = stats[i];

    rb_hash_aset(h_result, s_min, DBL2NUM(var_stats.min));
    rb_hash_aset(h_result, s_max, DBL2NUM(var_stats.max));
    rb_hash_aset(h_result, s_mean, DBL2NUM(var_stats.mean));
    rb_hash_aset(h_result, s_median, DBL2NUM(var_stats.median));
    rb_hash_aset(h_result, s_q1, DBL2NUM(var_stats.q1));
    rb_hash_aset(h_result, s_q3, DBL2NUM(var_stats.q3));
    rb_hash_aset(h_result, s_standard_deviation, DBL2NUM(var_stats.standard_deviation));
    rb_hash_aset(h_result, s_skew_median_pearson, DBL2NUM(var_stats.skew_median_pearson));

    rb_ary_push(a_results, h_result);
  }

  return a_results;
}

// Common
void
free_wrapped_array(void* dfloat)
{
  ((DFloat*)dfloat)->~DFloat();
  free(dfloat);
}

size_t
wrapped_array_size(const void* data)
{
  return sizeof(DFloat);
}

static rb_data_type_t dfloat_wrapper = [] {
  rb_data_type_t wrapper{};
  wrapper.wrap_struct_name = "dfloat";
  wrapper.function = { dmark : NULL, dfree : free_wrapped_array, dsize : wrapped_array_size };
  wrapper.data = NULL;
  wrapper.flags = RUBY_TYPED_FREE_IMMEDIATELY;
  return wrapper;
}();

VALUE
cArray2D_alloc(VALUE self)
{
  void* dfloat = (void*)malloc(sizeof(DFloat));

  return TypedData_Wrap_Struct(self, &dfloat_wrapper, dfloat);
}

/*
 * def initialize(arrays)
 */
VALUE
cArray2D_initialize(VALUE self, VALUE arrays)
{
  // Initialize dfloat structure to store Dfloat in type wrapper
  void* dfloat;
  UNWRAP_DFLOAT(self, dfloat);

  // Type-check 2D array
  if (TYPE(arrays) != T_ARRAY) {
    new (dfloat) DFloat(arrays, false);
    Check_Type(arrays, T_ARRAY);
    return false;
  }

  if (TYPE(rb_ary_entry(arrays, 0)) != T_ARRAY) {
    new (dfloat) DFloat(arrays, false);
    Check_Type(rb_ary_entry(arrays, 0), T_ARRAY);
    return false;
  }

  new (dfloat) DFloat(arrays, true);
  return self;
}

//{{{ Unpacked
VALUE
simd_disabled(VALUE self)
{
  return Qfalse;
}

/*
 * Unpacked descriptive statistics
 *
 * def descriptive_statistics
 */
VALUE
cArray2D_descriptive_statistics_unpacked(VALUE self)
{
  void* dfloat_untyped;
  UNWRAP_DFLOAT(self, dfloat_untyped);

  DFloat* dfloat = ((DFloat*)dfloat_untyped);
  Stats* stats = dfloat->descriptive_statistics();
  return build_results_hashes(stats, dfloat->cols);
}
//}}}

//{{{ Packed
#ifdef HAVE_XMMINTRIN_H
extern "C" VALUE
simd_enabled(VALUE self)
{
  return Qtrue;
}

/*
 * Packed descriptive statistics
 *
 * def descriptive_statistics
 */
VALUE
cArray2D_descriptive_statistics_packed(VALUE self)
{
  void* dfloat_untyped;
  UNWRAP_DFLOAT(self, dfloat_untyped);

  DFloat* dfloat = ((DFloat*)dfloat_untyped);
  Stats* stats = dfloat->descriptive_statistics_packed();
  return build_results_hashes(stats, dfloat->cols);
}
#endif
//}}}

extern "C" void
Init_fast_statistics(void)
{
  mFastStatistics = rb_define_module("FastStatistics");
  cArray2D = rb_define_class_under(mFastStatistics, "Array2D", rb_cObject);
  rb_define_alloc_func(cArray2D, cArray2D_alloc);
  rb_define_method(cArray2D, "initialize", RUBY_METHOD_FUNC(cArray2D_initialize), 1);

#ifdef HAVE_XMMINTRIN_H
  rb_define_singleton_method(mFastStatistics, "simd_enabled?", RUBY_METHOD_FUNC(simd_enabled), 0);
  rb_define_method(
    cArray2D,
    "descriptive_statistics",
    RUBY_METHOD_FUNC(cArray2D_descriptive_statistics_packed),
    0);
#else
  rb_define_singleton_method(mFastStatistics, "simd_enabled?", RUBY_METHOD_FUNC(simd_disabled), 0);
  rb_define_method(
    cArray2D,
    "descriptive_statistics",
    RUBY_METHOD_FUNC(cArray2D_descriptive_statistics_unpacked),
    0);
#endif
}
