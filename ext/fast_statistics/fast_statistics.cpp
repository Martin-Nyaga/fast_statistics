#include "fast_statistics.h"
#include "debug.h"

using namespace array_2d;

#define rb_sym(str) ID2SYM(rb_intern(str))
#define UNWRAP_DFLOAT(obj, var) TypedData_Get_Struct((obj), void*, &dfloat_wrapper, (var));

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

// Common
void
free_wrapped_array(void* array)
{
  // It's okay to cast to a DFloat64Unpacked becuase all DFloats have the same
  // size & memory layout
  ((DFloat64Unpacked*)array)->~DFloat64Unpacked();
}

size_t
wrapped_array_size(const void* data)
{
  // It's okay to cast to a DFloat64Unpacked becuase all DFloats have the same
  // size & memory layout
  DFloat64Unpacked* array = (DFloat64Unpacked*)data;
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

VALUE
cArray2D_alloc(VALUE self)
{
  void* dfloat = (void*)malloc(sizeof(void*));

  return TypedData_Wrap_Struct(self, &dfloat_wrapper, dfloat);
}

void
cArray2D_initialize_parse_arguments(
  VALUE argc,
  VALUE* argv,
  VALUE* arrays,
  VALUE* dtype,
  VALUE* packed)
{
  VALUE opts;
  rb_scan_args(argc, argv, "1:", arrays, &opts);

  if (NIL_P(opts)) opts = rb_hash_new();

  // default dtype: :double
  *dtype = rb_hash_aref(opts, rb_sym("dtype"));
  if (*dtype == Qnil) *dtype = rb_sym("double");

  // Typecheck dtype
  if (!(*dtype == rb_sym("float") || *dtype == rb_sym("double")))
    rb_raise(rb_eTypeError, "Expected dtype: to be one of :float, :double");

  // default packed: true
  *packed = rb_hash_aref(opts, rb_sym("packed"));
  if (*packed == Qnil) *packed = Qtrue;

  // Typecheck packed
  if (!(*packed == Qtrue || *packed == Qfalse))
    rb_raise(rb_eTypeError, "Expected packed: to be a boolean");

  // Typecheck 2d array
  Check_Type(*arrays, T_ARRAY);
  Check_Type(rb_ary_entry(*arrays, 0), T_ARRAY);
}

//{{{ Unpacked
VALUE
simd_disabled(VALUE self)
{
  return Qfalse;
}

VALUE
cArray2D_initialize_data_unpacked(VALUE self, VALUE arrays, VALUE dtype)
{
  // Initialize dfloat structure to store Dfloat in type wrapper
  void* dfloat;
  UNWRAP_DFLOAT(self, dfloat);

  if (dtype == rb_sym("float")) {
    new (dfloat) DFloat32Unpacked(arrays);
  } else {
    new (dfloat) DFloat64Unpacked(arrays);
  }

  return self;
}

VALUE
cArray2D_initialize_unpacked(VALUE argc, VALUE* argv, VALUE self)
{
  VALUE arrays, dtype, packed;
  cArray2D_initialize_parse_arguments(argc, argv, &arrays, &dtype, &packed);

  rb_ivar_set(self, rb_intern("@dtype"), dtype);
  rb_ivar_set(self, rb_intern("@packed"), packed);

  return cArray2D_initialize_data_unpacked(self, arrays, dtype);
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

  VALUE dtype = rb_ivar_get(self, rb_intern("@dtype"));

  if (dtype == rb_sym("float")) {
    DFloat32Unpacked* dfloat = ((DFloat32Unpacked*)dfloat_untyped);
    Stats* stats = dfloat->descriptive_statistics();
    return build_results_hashes(stats, dfloat->cols);
  } else {
    DFloat64Unpacked* dfloat = ((DFloat64Unpacked*)dfloat_untyped);
    Stats* stats = dfloat->descriptive_statistics();
    return build_results_hashes(stats, dfloat->cols);
  }
}

/*
 * Unpacked mean computation
 *
 * def mean
 */
VALUE
cArray2D_mean_unpacked(VALUE self)
{
  VALUE dtype = rb_ivar_get(self, rb_intern("@dtype"));

  void* dfloat_untyped;
  UNWRAP_DFLOAT(self, dfloat_untyped);

  if (dtype == rb_sym("float")) {
    DFloat32Unpacked* dfloat = ((DFloat32Unpacked*)dfloat_untyped);
    double* means = dfloat->mean();

    VALUE means_arr = rb_ary_new();
    for (int i = 0; i < dfloat->cols; i++) {
      rb_ary_push(means_arr, DBL2NUM((double)means[i]));
    }
    free(means);
    return means_arr;
  } else {
    DFloat64Unpacked* dfloat = ((DFloat64Unpacked*)dfloat_untyped);
    double* means = dfloat->mean();

    VALUE means_arr = rb_ary_new();
    for (int i = 0; i < dfloat->cols; i++) {
      rb_ary_push(means_arr, DBL2NUM((double)means[i]));
    }
    free(means);
    return means_arr;
  }
}
//}}}

// Packed
#ifdef HAVE_XMMINTRIN_H
extern "C" VALUE
simd_enabled(VALUE self)
{
  return Qtrue;
}

VALUE
cArray2D_initialize_data_packed(VALUE self, VALUE arrays, VALUE dtype)
{
  // Initialize dfloat structure to store Dfloat in type wrapper
  void* dfloat;
  UNWRAP_DFLOAT(self, dfloat);

  if (dtype == rb_sym("float")) {
    new (dfloat) DFloat32Packed(arrays);
  } else {
    new (dfloat) DFloat64Packed(arrays);
  }

  return self;
}

/*
 * def initialize(arrays, dtype: :double, packed: true)
 */
VALUE
cArray2D_initialize_packed(VALUE argc, VALUE* argv, VALUE self)
{
  VALUE arrays, dtype, packed;
  cArray2D_initialize_parse_arguments(argc, argv, &arrays, &dtype, &packed);

  rb_ivar_set(self, rb_intern("@dtype"), dtype);
  rb_ivar_set(self, rb_intern("@packed"), packed);

  if (packed == Qfalse) {
    return cArray2D_initialize_data_unpacked(self, arrays, dtype);
  } else {
    return cArray2D_initialize_data_packed(self, arrays, dtype);
  }
}

/*
 * Packed descriptive statistics
 *
 * def descriptive_statistics
 */
VALUE
cArray2D_descriptive_statistics_packed(VALUE self)
{
  VALUE packed = rb_ivar_get(self, rb_intern("@packed"));
  if (packed == Qfalse) {
    return cArray2D_descriptive_statistics_unpacked(self);
  }

  VALUE dtype = rb_ivar_get(self, rb_intern("@dtype"));

  void* dfloat_untyped;
  UNWRAP_DFLOAT(self, dfloat_untyped);

  if (dtype == rb_sym("float")) {
    DFloat32Packed* dfloat = ((DFloat32Packed*)dfloat_untyped);
    Stats* stats = dfloat->descriptive_statistics_simd();
    return build_results_hashes(stats, dfloat->cols);
  } else {
    DFloat64Packed* dfloat = ((DFloat64Packed*)dfloat_untyped);
    Stats* stats = dfloat->descriptive_statistics_simd();
    return build_results_hashes(stats, dfloat->cols);
  }
}

/*
 * Packed mean computation
 *
 * def mean
 */
VALUE
cArray2D_mean_packed(VALUE self)
{
  PROFILE;

  VALUE result;
  VALUE packed = rb_ivar_get(self, rb_intern("@packed"));
  if (packed == Qfalse) {
    result = cArray2D_mean_unpacked(self);
  }

  VALUE dtype = rb_ivar_get(self, rb_intern("@dtype"));

  void* dfloat_untyped;
  UNWRAP_DFLOAT(self, dfloat_untyped);

  if (dtype == rb_sym("float")) {
    DFloat32Packed* dfloat = ((DFloat32Packed*)dfloat_untyped);
    double* means = dfloat->mean();

    VALUE means_arr = rb_ary_new();
    for (int i = 0; i < dfloat->cols; i++) {
      rb_ary_push(means_arr, DBL2NUM(means[i]));
    }
    free(means);
    result = means_arr;
  } else {
    DFloat64Packed* dfloat = ((DFloat64Packed*)dfloat_untyped);
    double* means = dfloat->mean();

    VALUE means_arr = rb_ary_new();
    for (int i = 0; i < dfloat->cols; i++) {
      rb_ary_push(means_arr, DBL2NUM(means[i]));
    }
    free(means);
    result = means_arr;
  }

  return result;
}
#endif

extern "C" void
Init_fast_statistics(void)
{
  mFastStatistics = rb_define_module("FastStatistics");
  cArray2D = rb_define_class_under(mFastStatistics, "Array2D", rb_cData);
  rb_define_alloc_func(cArray2D, cArray2D_alloc);

#ifdef HAVE_XMMINTRIN_H
  rb_define_singleton_method(mFastStatistics, "simd_enabled?", RUBY_METHOD_FUNC(simd_enabled), 0);
  rb_define_method(cArray2D, "initialize", RUBY_METHOD_FUNC(cArray2D_initialize_packed), -1);
  rb_define_method(
    cArray2D,
    "descriptive_statistics",
    RUBY_METHOD_FUNC(cArray2D_descriptive_statistics_packed),
    0);
  rb_define_method(cArray2D, "mean", RUBY_METHOD_FUNC(cArray2D_mean_packed), 0);
#else
  rb_define_singleton_method(mFastStatistics, "simd_enabled?", RUBY_METHOD_FUNC(simd_disabled), 0);
  rb_define_method(cArray2D, "initialize", RUBY_METHOD_FUNC(cArray2D_initialize_unpacked), -1);
  rb_define_method(
    cArray2D,
    "descriptive_statistics",
    RUBY_METHOD_FUNC(cArray2D_descriptive_statistics_unpacked),
    0);
#endif
}
