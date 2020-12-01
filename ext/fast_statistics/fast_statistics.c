#include "fast_statistics.h"

static VALUE mFastStatistics;

#ifdef HAVE_XMMINTRIN_H
#define mm_get_index_float(packed, index) *(((float *)&packed) + index);
#define mm_get_index_double(packed, index) *(((double *)&packed) + index);

inline double safe_array_entry_f64_from_columns(VALUE cols[], int col_index,
                                                int row_index)
{
  double result = 0.0;
  if (cols[col_index] != Qnil) {
    result = NUM2DBL(rb_ary_entry(cols[col_index], row_index));
  }
  return result;
}

inline float safe_array_entry_f32_from_columns(VALUE cols[], int col_index,
                                               int row_index)
{
  float result = 0.0f;
  if (cols[col_index] != Qnil) {
    result = (float)NUM2DBL(rb_ary_entry(cols[col_index], row_index));
  }
  return result;
}

inline __m128d pack_float64_m128(VALUE cols[], int row_index)
{
  __m128d packed =
      _mm_set_pd(safe_array_entry_f64_from_columns(cols, 0, row_index),
                 safe_array_entry_f64_from_columns(cols, 1, row_index));
  return packed;
}

inline __m128 pack_float32_m128(VALUE cols[], int row_index)
{
  __m128 packed =
      _mm_set_ps(safe_array_entry_f32_from_columns(cols, 0, row_index),
                 safe_array_entry_f32_from_columns(cols, 1, row_index),
                 safe_array_entry_f32_from_columns(cols, 2, row_index),
                 safe_array_entry_f32_from_columns(cols, 3, row_index));
  return packed;
}

// Pack values in opposite order to maintain expected ruby order
inline void get_next_columns(VALUE current_cols[], int simd_pack_size,
                             int variable_index, int cols, VALUE arrays)
{
  for (int simd_slot_index = 0; simd_slot_index < simd_pack_size;
       simd_slot_index++) {
    int col_index = simd_pack_size - simd_slot_index - 1;
    if ((variable_index + simd_slot_index) < cols) {
      current_cols[col_index] =
          rb_ary_entry(arrays, variable_index + simd_slot_index);
    } else {
      current_cols[col_index] = Qnil;
    }
  };
}

VALUE
descriptive_statistics_packed_float64(VALUE self, VALUE arrays)
{
  Check_Type(arrays, T_ARRAY);

  int cols = rb_array_len(arrays);
  int rows = rb_array_len(rb_ary_entry(arrays, 0));
  const int simd_pack_size = 2;

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_variance = ID2SYM(rb_intern("variance"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  for (int variable_index = 0; variable_index < cols;
       variable_index += simd_pack_size) {
    VALUE current_cols[simd_pack_size];
    get_next_columns(current_cols, simd_pack_size, variable_index, cols,
                     arrays);

    __m128d sums = _mm_setzero_pd();
    __m128d means = _mm_setzero_pd();
    __m128d mins = _mm_set_pd1(FLT_MAX);
    __m128d maxes = _mm_set_pd1(FLT_MIN);
    __m128d variances = _mm_setzero_pd();
    __m128d standard_deviations = _mm_setzero_pd();
    __m128d lengths = _mm_set_pd1((float)rows);

    for (int row_index = 0; row_index < rows; row_index++) {
      __m128d packed = pack_float64_m128(current_cols, row_index);
      sums = _mm_add_pd(sums, packed);
      mins = _mm_min_pd(mins, packed);
      maxes = _mm_max_pd(maxes, packed);
    }
    means = _mm_div_pd(sums, lengths);

    for (int row_index = 0; row_index < rows; row_index++) {
      __m128d packed = pack_float64_m128(current_cols, row_index);
      __m128d deviation = _mm_sub_pd(packed, means);
      __m128d sqr_deviation = _mm_mul_pd(deviation, deviation);
      variances = _mm_add_pd(variances, _mm_div_pd(sqr_deviation, lengths));
    }
    standard_deviations = _mm_sqrt_pd(variances);

    for (int simd_slot_index = 0; simd_slot_index < simd_pack_size;
         simd_slot_index++) {
      if ((variable_index + simd_slot_index) < cols) {
        VALUE h_result = rb_hash_new();

        double min = mm_get_index_double(mins, simd_slot_index);
        double max = mm_get_index_double(maxes, simd_slot_index);
        double mean = mm_get_index_double(means, simd_slot_index);
        double variance = mm_get_index_double(variances, simd_slot_index);
        double standard_deviation =
            mm_get_index_double(standard_deviations, simd_slot_index);

        rb_hash_aset(h_result, s_min, DBL2NUM(min));
        rb_hash_aset(h_result, s_max, DBL2NUM(max));
        rb_hash_aset(h_result, s_mean, DBL2NUM(mean));
        rb_hash_aset(h_result, s_variance, DBL2NUM(variance));
        rb_hash_aset(h_result, s_standard_deviation,
                     DBL2NUM(standard_deviation));

        rb_ary_push(a_results, h_result);
      }
    }
  }

  return a_results;
}

VALUE descriptive_statistics_packed_float32(VALUE self, VALUE arrays)
{
  Check_Type(arrays, T_ARRAY);

  int cols = rb_array_len(arrays);
  int rows = rb_array_len(rb_ary_entry(arrays, 0));
  const int simd_pack_size = 4;

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_variance = ID2SYM(rb_intern("variance"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  for (int variable_index = 0; variable_index < cols;
       variable_index += simd_pack_size) {
    VALUE current_cols[simd_pack_size];
    get_next_columns(current_cols, simd_pack_size, variable_index, cols,
                     arrays);

    __m128 sums = _mm_setzero_ps();
    __m128 means = _mm_setzero_ps();
    __m128 mins = _mm_set_ps1(FLT_MAX);
    __m128 maxes = _mm_set_ps1(FLT_MIN);
    __m128 variances = _mm_setzero_ps();
    __m128 standard_deviations = _mm_setzero_ps();
    __m128 lengths = _mm_set_ps1((float)rows);

    for (int row_index = 0; row_index < rows; row_index++) {
      __m128 packed = pack_float32_m128(current_cols, row_index);
      sums = _mm_add_ps(sums, packed);
      mins = _mm_min_ps(mins, packed);
      maxes = _mm_max_ps(maxes, packed);
    }
    means = _mm_div_ps(sums, lengths);

    for (int row_index = 0; row_index < rows; row_index++) {
      __m128 packed = pack_float32_m128(current_cols, row_index);
      __m128 deviation = _mm_sub_ps(packed, means);
      __m128 sqr_deviation = _mm_mul_ps(deviation, deviation);
      variances = _mm_add_ps(variances, _mm_div_ps(sqr_deviation, lengths));
    }
    standard_deviations = _mm_sqrt_ps(variances);

    for (int simd_slot_index = 0; simd_slot_index < simd_pack_size;
         simd_slot_index++) {
      if ((variable_index + simd_slot_index) < cols) {
        VALUE h_result = rb_hash_new();

        double min = mm_get_index_float(mins, simd_slot_index);
        double max = mm_get_index_float(maxes, simd_slot_index);
        double mean = mm_get_index_float(means, simd_slot_index);
        double variance = mm_get_index_float(variances, simd_slot_index);
        double standard_deviation =
            mm_get_index_float(standard_deviations, simd_slot_index);

        rb_hash_aset(h_result, s_min, DBL2NUM(min));
        rb_hash_aset(h_result, s_max, DBL2NUM(max));
        rb_hash_aset(h_result, s_mean, DBL2NUM(mean));
        rb_hash_aset(h_result, s_variance, DBL2NUM(variance));
        rb_hash_aset(h_result, s_standard_deviation,
                     DBL2NUM(standard_deviation));

        rb_ary_push(a_results, h_result);
      }
    }
  }

  return a_results;
}

VALUE simd_enabled(VALUE self) { return Qtrue; }
#endif

VALUE descriptive_statistics(VALUE self, VALUE arrays)
{
  Check_Type(arrays, T_ARRAY);

  int cols = rb_array_len(arrays);
  int rows = rb_array_len(rb_ary_entry(arrays, 0));

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_variance = ID2SYM(rb_intern("variance"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  for (int variable_index = 0; variable_index < cols; variable_index++) {
    VALUE col = rb_ary_entry(arrays, variable_index);
    Check_Type(col, T_ARRAY);
    VALUE h_result = rb_hash_new();

    double sum = 0.0f;
    double min = FLT_MAX;
    double max = FLT_MIN;

    for (int i = 0; i < rows; i++) {
      VALUE value = rb_ary_entry(col, i);
      Check_Type(value, T_FLOAT);
      double valuef64 = RFLOAT_VALUE(value);

      sum += valuef64;
      if (valuef64 < min) min = valuef64;
      if (valuef64 > max) max = valuef64;
    }
    double mean = sum / rows;

    double variance = 0.0f;
    for (int i = 0; i < rows; i++) {
      VALUE value = rb_ary_entry(col, i);
      double valuef64 = NUM2DBL(value);

      double deviation = valuef64 - mean;
      double sqr_deviation = deviation * deviation;
      variance += (sqr_deviation / rows);
    }
    double standard_deviation = sqrt(variance);

    rb_hash_aset(h_result, s_min, DBL2NUM(min));
    rb_hash_aset(h_result, s_max, DBL2NUM(max));
    rb_hash_aset(h_result, s_mean, DBL2NUM(mean));
    rb_hash_aset(h_result, s_variance, DBL2NUM(variance));
    rb_hash_aset(h_result, s_standard_deviation, DBL2NUM(standard_deviation));

    rb_ary_push(a_results, h_result);
  }

  return a_results;
}

VALUE simd_disabled(VALUE self) { return Qfalse; }

void Init_fast_statistics(void)
{
  mFastStatistics = rb_define_module("FastStatistics");

#ifdef HAVE_XMMINTRIN_H
  // Provide double precision packed as default
  rb_define_singleton_method(mFastStatistics, "descriptive_statistics",
                             descriptive_statistics_packed_float64, 1);
  rb_define_singleton_method(mFastStatistics,
                             "descriptive_statistics_packed_float32",
                             descriptive_statistics_packed_float32, 1);
  rb_define_singleton_method(mFastStatistics,
                             "descriptive_statistics_packed_float64",
                             descriptive_statistics_packed_float64, 1);
  rb_define_singleton_method(mFastStatistics, "descriptive_statistics_unpacked",
                             descriptive_statistics, 1);
  rb_define_singleton_method(mFastStatistics, "simd_enabled?", simd_enabled, 0);
#else
  // No SIMD support:
  // Provide all "packed" and "unpacked" versions as the same algorithm
  rb_define_singleton_method(mFastStatistics, "descriptive_statistics",
                             descriptive_statistics, 1);
  rb_define_singleton_method(mFastStatistics,
                             "descriptive_statistics_packed_float32",
                             descriptive_statistics, 1);
  rb_define_singleton_method(mFastStatistics,
                             "descriptive_statistics_packed_float64",
                             descriptive_statistics, 1);
  rb_define_singleton_method(mFastStatistics, "descriptive_statistics_unpacked",
                             descriptive_statistics, 1);
  rb_define_singleton_method(mFastStatistics, "simd_enabled?", simd_disabled,
                             0);
#endif
}
