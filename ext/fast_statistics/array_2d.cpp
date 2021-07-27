#include "array_2d.h"

namespace array_2d {

static VALUE rb_c_array_2d;

void free_wrapped_array(void *dfloat) {
  ((DFloat *)dfloat)->~DFloat();
  free(dfloat);
}

size_t wrapped_array_size(const void *data) { return sizeof(DFloat); }

const rb_data_type_t dfloat_wrapper = [] {
  rb_data_type_t wrapper;
  wrapper.wrap_struct_name = "dfloat";
  wrapper.function = {.dmark = NULL, .dfree = free_wrapped_array, .dsize = wrapped_array_size};
  wrapper.data = NULL;
  wrapper.flags = RUBY_TYPED_FREE_IMMEDIATELY;
  return wrapper;
}();

VALUE rb_alloc(VALUE self) {
  void *dfloat = (void *)malloc(sizeof(DFloat));
  return TypedData_Wrap_Struct(self, &dfloat_wrapper, dfloat);
}

/*
 * def initialize(arrays)
 */
VALUE rb_initialize(VALUE self, VALUE arrays) {
  // Get untyped memory for a dfloat
  GET_DFLOAT_UNTYPED(self, dfloat);

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

VALUE build_results_hashes(Stats *stats, int num_variables) {
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

/*
 * def descriptive_statistics
 */
VALUE rb_descriptive_statistics(VALUE self) {
  GET_DFLOAT(self, dfloat);
  Stats *stats = dfloat->descriptive_statistics();
  return build_results_hashes(stats, dfloat->cols);
}

VALUE to_ruby_array(double *c_array, int count) {
  VALUE ruby_array = rb_ary_new();
  for (auto i = 0; i < count; i++) {
    rb_ary_push(ruby_array, DBL2NUM(c_array[i]));
  }
  return ruby_array;
}

VALUE rb_mean(VALUE self) {
  GET_DFLOAT(self, dfloat);
  double *means = dfloat->mean();
  return to_ruby_array(means, dfloat->cols);
}

void setup(VALUE rb_m_fast_statistics) {
  rb_c_array_2d = rb_define_class_under(rb_m_fast_statistics, "Array2D", rb_cData);
  rb_define_alloc_func(rb_c_array_2d, rb_alloc);
  rb_define_method(rb_c_array_2d, "initialize", RUBY_METHOD_FUNC(rb_initialize), 1);
  rb_define_method(
      rb_c_array_2d, "descriptive_statistics", RUBY_METHOD_FUNC(rb_descriptive_statistics), 0);
  rb_define_method(rb_c_array_2d, "mean", RUBY_METHOD_FUNC(rb_mean), 0);
}
} // namespace array_2d
