#include "fast_statistics.h"

using namespace array_2d;
static VALUE mFastStatistics;
static VALUE cArray2D;

#define rb_sym(str) ID2SYM(rb_intern(str))

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

    rb_ary_push(a_results, h_result);
  }

  return a_results;
}

#ifdef HAVE_XMMINTRIN_H

extern "C" VALUE
descriptive_statistics_packed_float64(VALUE self, VALUE arrays)
{
  DFloat<double, Packed64>* array = new DFloat<double, Packed64>(arrays);

  Stats* stats = array->descriptive_statistics_simd();
  VALUE a_results = build_results_hashes(stats, array->cols);

  delete array;
  return a_results;
}

extern "C" VALUE
descriptive_statistics_packed_float32(VALUE self, VALUE arrays)
{
  DFloat<float, Packed32>* array = new DFloat<float, Packed32>(arrays);

  Stats* stats = array->descriptive_statistics_simd();
  VALUE a_results = build_results_hashes(stats, array->cols);

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
descriptive_statistics_unpacked(VALUE self, VALUE arrays)
{
  DFloat<double, Unpacked>* array = new DFloat<double, Unpacked>(arrays);

  Stats* stats = array->descriptive_statistics();
  VALUE a_results = build_results_hashes(stats, array->cols);

  delete array;
  return a_results;
}

extern "C" VALUE
simd_disabled(VALUE self)
{
  return Qfalse;
}

void
free_wrapped_array(void* array)
{
  ((DFloat64Unpacked*)array)->~DFloat<double, Unpacked>();
}

size_t
wrapped_array_size(const void* data)
{
  DFloat<double, Unpacked>* array = (DFloat<double, Unpacked>*)data;
  size_t size = sizeof(array->entries) + sizeof(*array);
  return size;
}

static const rb_data_type_t dfloat_wrapper = {
	.wrap_struct_name = "dfloat",
	.function = {
		.dmark = NULL,
		.dfree = free_wrapped_array,
		.dsize = wrapped_array_size,
	},
	.data = NULL,
	.flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

#define UNWRAP_DFLOAT(obj, var) TypedData_Get_Struct((obj), void*, &dfloat_wrapper, (var));

/*
 * def initialize(arrays, dtype: :float, packed: true)
 */
VALUE
cArray2D_initialize(VALUE argc, VALUE* argv, VALUE self)
{
  VALUE arrays, opts, dtype, packed;
  rb_scan_args(argc, argv, "1:", &arrays, &opts);
  if (NIL_P(opts)) opts = rb_hash_new();

  dtype = rb_hash_aref(opts, rb_sym("dtype"));
  if (dtype == Qnil) dtype = rb_sym("double");
  packed = rb_hash_aref(opts, rb_sym("packed"));
  if (packed == Qnil) packed = Qtrue;

  Check_Type(arrays, T_ARRAY);
  Check_Type(rb_ary_entry(arrays, 0), T_ARRAY);
  Check_Type(dtype, T_SYMBOL);
  if (!(packed == Qtrue || packed == Qfalse))
    rb_raise(rb_eTypeError, "Expected packed: to be a boolean");

  // Initialize dfloat structure
  void* dfloat;
  UNWRAP_DFLOAT(self, dfloat);

  rb_ivar_set(self, rb_intern("@dtype"), dtype);
  rb_ivar_set(self, rb_intern("@packed"), packed);

  if (dtype == rb_sym("float")) {
    if (packed == Qtrue) {
      new (dfloat) DFloat32Packed(arrays);
    } else {
      new (dfloat) DFloat32Unpacked(arrays);
    }
  } else if (dtype == rb_sym("double")) {
    if (packed == Qtrue) {
      new (dfloat) DFloat64Packed(arrays);
    } else {
      new (dfloat) DFloat64Unpacked(arrays);
    }
  } else {
    rb_raise(rb_eTypeError, "Expected :dtype to be one of :float, :double");
  }

  return self;
}

VALUE
cArray2D_alloc(VALUE self)
{
  void* dfloat = (void*)malloc(sizeof(void*));

  return TypedData_Wrap_Struct(self, &dfloat_wrapper, dfloat);
}

VALUE
cArray2D_descriptive_statistics(VALUE self)
{
  void* dfloat_untyped;
  UNWRAP_DFLOAT(self, dfloat_untyped);

  VALUE packed = rb_ivar_get(self, rb_intern("@packed"));
  VALUE dtype = rb_ivar_get(self, rb_intern("@dtype"));

  if (dtype == rb_sym("float")) {
    if (packed == Qtrue) {
      DFloat32Packed* dfloat = ((DFloat32Packed*)dfloat_untyped);
      Stats* stats = dfloat->descriptive_statistics();
      return build_results_hashes(stats, dfloat->cols);
    } else {
      DFloat32Unpacked* dfloat = ((DFloat32Unpacked*)dfloat_untyped);
      Stats* stats = dfloat->descriptive_statistics();
      return build_results_hashes(stats, dfloat->cols);
    }
  } else if (dtype == rb_sym("double")) {
    if (packed == Qtrue) {
      DFloat64Packed* dfloat = ((DFloat64Packed*)dfloat_untyped);
      Stats* stats = dfloat->descriptive_statistics();
      return build_results_hashes(stats, dfloat->cols);
    } else {
      DFloat64Unpacked* dfloat = ((DFloat64Unpacked*)dfloat_untyped);
      Stats* stats = dfloat->descriptive_statistics();
      return build_results_hashes(stats, dfloat->cols);
    }
  } else {
    rb_raise(rb_eTypeError, "Expected :dtype to be one of :float, :double");
  }
}

extern "C" void
Init_fast_statistics(void)
{
  mFastStatistics = rb_define_module("FastStatistics");
  cArray2D = rb_define_class_under(mFastStatistics, "Array2D", rb_cData);
  rb_define_alloc_func(cArray2D, cArray2D_alloc);
  rb_define_method(cArray2D, "initialize", RUBY_METHOD_FUNC(cArray2D_initialize), -1);
  rb_define_method(
    cArray2D, "descriptive_statistics", RUBY_METHOD_FUNC(cArray2D_descriptive_statistics), 0);

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
      RUBY_METHOD_FUNC(descriptive_statistics_unpacked), 1);
  rb_define_singleton_method(
      mFastStatistics, "simd_enabled?",
      RUBY_METHOD_FUNC(simd_enabled), 0);
#else
  // No SIMD support:
  // Provide all "packed" and "unpacked" versions as the same algorithm
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics",
      RUBY_METHOD_FUNC(descriptive_statistics_unpacked), 1);
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics_packed_float32",
      RUBY_METHOD_FUNC(descriptive_statistics_unpacked), 1);
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics_packed_float64",
      RUBY_METHOD_FUNC(descriptive_statistics_unpacked), 1);
  rb_define_singleton_method(
      mFastStatistics, "descriptive_statistics_unpacked",
      RUBY_METHOD_FUNC(descriptive_statistics_unpacked), 1);
  rb_define_singleton_method(
      mFastStatistics, "simd_enabled?",
      RUBY_METHOD_FUNC(simd_disabled), 0);
#endif
  // clang-format on
}
