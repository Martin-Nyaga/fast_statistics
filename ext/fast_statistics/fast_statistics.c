#include "fast_statistics.h"

static VALUE mFastStatistics;

#ifdef HAVE_XMMINTRIN_H
#define MM_GET_INDEX_FLOAT(packed, index) *(((float *)&packed) + index);
#define MM_GET_INDEX_DOUBLE(packed, index) *(((double *)&packed) + index);

inline double safe_array_entry_f64_from_columns(VALUE arrays[], int col_index,
                                                int row_index)
{
  double result = 0.0;
  if (arrays[col_index] != Qnil) {
    result = NUM2DBL(rb_ary_entry(arrays[col_index], row_index));
  }
  return result;
}

inline float safe_array_entry_f32_from_columns(VALUE arrays[], int col_index,
                                               int row_index)
{
  float result = 0.0f;
  if (arrays[col_index] != Qnil) {
    result = (float)NUM2DBL(rb_ary_entry(arrays[col_index], row_index));
  }
  return result;
}

inline __m128d pack_float64(VALUE arrays[], int row_index)
{
  __m128d packed =
      _mm_set_pd(safe_array_entry_f64_from_columns(arrays, 0, row_index),
                 safe_array_entry_f64_from_columns(arrays, 1, row_index));
  return packed;
}

inline __m128 pack_float32(VALUE arrays[], int row_index)
{
  __m128 packed =
      _mm_set_ps(safe_array_entry_f32_from_columns(arrays, 0, row_index),
                 safe_array_entry_f32_from_columns(arrays, 1, row_index),
                 safe_array_entry_f32_from_columns(arrays, 2, row_index),
                 safe_array_entry_f32_from_columns(arrays, 3, row_index));
  return packed;
}

// Pack column values from arrays in opposite order to maintain expected ruby
// order
inline void get_next_columns(VALUE current_cols[], int simd_pack_size,
                             int base_variable_index, int cols, VALUE arrays)
{
  for (int simd_slot_index = 0; simd_slot_index < simd_pack_size;
       simd_slot_index++) {
    int col_index = simd_pack_size - simd_slot_index - 1;
    if ((base_variable_index + simd_slot_index) < cols) {
      current_cols[col_index] =
          rb_ary_entry(arrays, base_variable_index + simd_slot_index);
    } else {
      current_cols[col_index] = Qnil;
    }
  };
}

inline __m128d array_percentile_packed_float64(VALUE sorted_arrays[],
                                               double pct, int length)
{
  if (pct == 1.0) return pack_float64(sorted_arrays, length - 1);
  double rank = pct * (double)(length - 1);
  int floored_rank = floor(rank);
  __m128d lower = pack_float64(sorted_arrays, floored_rank);
  __m128d upper = pack_float64(sorted_arrays, floored_rank + 1);
  return _mm_add_pd(lower,
                    _mm_mul_pd(_mm_sub_pd(upper, lower),
                               _mm_sub_pd(_mm_set_pd1(rank),
                                          _mm_set_pd1((double)floored_rank))));
}

inline __m128 array_percentile_packed_float32(VALUE sorted_arrays[], float pct,
                                              int length)
{
  if (pct == 1.0) return pack_float32(sorted_arrays, length - 1);
  float rank = pct * (float)(length - 1);
  int floored_rank = floor(rank);
  __m128 lower = pack_float32(sorted_arrays, floored_rank);
  __m128 upper = pack_float32(sorted_arrays, floored_rank + 1);
  return _mm_add_ps(lower,
                    _mm_mul_ps(_mm_sub_ps(upper, lower),
                               _mm_sub_ps(_mm_set_ps1(rank),
                                          _mm_set_ps1((float)floored_rank))));
}

inline void sort_arrays(VALUE arrays[], int array_count)
{
  for (int index = 0; index < array_count; index++) {
    if (arrays[index] != Qnil) {
      rb_ary_sort_bang(arrays[index]);
    }
  }
}

VALUE
descriptive_statistics_packed_float64(VALUE self, VALUE arrays)
{
  Check_Type(arrays, T_ARRAY);

  int array_count = rb_array_len(arrays);
  int values_count = rb_array_len(rb_ary_entry(arrays, 0));
  const int simd_pack_size = 2;

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_median = ID2SYM(rb_intern("median"));
  VALUE s_q1 = ID2SYM(rb_intern("q1"));
  VALUE s_q3 = ID2SYM(rb_intern("q3"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  __m128d lengths = _mm_set_pd1((double)values_count);
  for (int variable_index = 0; variable_index < array_count;
       variable_index += simd_pack_size) {
    VALUE current_cols[simd_pack_size];
    get_next_columns(current_cols, simd_pack_size, variable_index, array_count,
                     arrays);

    sort_arrays(current_cols, simd_pack_size);
    __m128d mins = pack_float64(current_cols, 0);
    __m128d maxes = pack_float64(current_cols, values_count - 1);

    __m128d sums = _mm_setzero_pd();
    for (int row_index = 0; row_index < values_count; row_index++) {
      __m128d packed = pack_float64(current_cols, row_index);
      sums = _mm_add_pd(sums, packed);
    }
    __m128d means = _mm_div_pd(sums, lengths);

    __m128d medians =
        array_percentile_packed_float64(current_cols, 0.5, values_count);
    __m128d q1s =
        array_percentile_packed_float64(current_cols, 0.25, values_count);
    __m128d q3s =
        array_percentile_packed_float64(current_cols, 0.75, values_count);

    __m128d variances = _mm_setzero_pd();
    for (int row_index = 0; row_index < values_count; row_index++) {
      __m128d packed = pack_float64(current_cols, row_index);
      __m128d deviation = _mm_sub_pd(packed, means);
      __m128d sqr_deviation = _mm_mul_pd(deviation, deviation);
      variances = _mm_add_pd(variances, _mm_div_pd(sqr_deviation, lengths));
    }
    __m128d stdevs = _mm_sqrt_pd(variances);

    for (int simd_slot_index = 0; simd_slot_index < simd_pack_size;
         simd_slot_index++) {
      if ((variable_index + simd_slot_index) < array_count) {
        VALUE h_result = rb_hash_new();

        double min = MM_GET_INDEX_DOUBLE(mins, simd_slot_index);
        double max = MM_GET_INDEX_DOUBLE(maxes, simd_slot_index);
        double mean = MM_GET_INDEX_DOUBLE(means, simd_slot_index);
        double median = MM_GET_INDEX_DOUBLE(medians, simd_slot_index);
        double q1 = MM_GET_INDEX_DOUBLE(q1s, simd_slot_index);
        double q3 = MM_GET_INDEX_DOUBLE(q3s, simd_slot_index);
        double stdev = MM_GET_INDEX_DOUBLE(stdevs, simd_slot_index);

        rb_hash_aset(h_result, s_min, DBL2NUM(min));
        rb_hash_aset(h_result, s_max, DBL2NUM(max));
        rb_hash_aset(h_result, s_mean, DBL2NUM(mean));
        rb_hash_aset(h_result, s_median, DBL2NUM(median));
        rb_hash_aset(h_result, s_q1, DBL2NUM(q1));
        rb_hash_aset(h_result, s_q3, DBL2NUM(q3));
        rb_hash_aset(h_result, s_standard_deviation, DBL2NUM(stdev));

        rb_ary_push(a_results, h_result);
      }
    }
  }

  return a_results;
}

VALUE descriptive_statistics_packed_float32(VALUE self, VALUE arrays)
{
  Check_Type(arrays, T_ARRAY);

  int array_count = rb_array_len(arrays);
  int values_count = rb_array_len(rb_ary_entry(arrays, 0));
  const int simd_pack_size = 4;

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_median = ID2SYM(rb_intern("median"));
  VALUE s_q1 = ID2SYM(rb_intern("q1"));
  VALUE s_q3 = ID2SYM(rb_intern("q3"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  __m128 lengths = _mm_set_ps1((float)values_count);
  for (int variable_index = 0; variable_index < array_count;
       variable_index += simd_pack_size) {
    VALUE current_cols[simd_pack_size];
    get_next_columns(current_cols, simd_pack_size, variable_index, array_count,
                     arrays);

    sort_arrays(current_cols, simd_pack_size);
    __m128 mins = pack_float32(current_cols, 0);
    __m128 maxes = pack_float32(current_cols, values_count - 1);

    __m128 sums = _mm_setzero_ps();
    for (int row_index = 0; row_index < values_count; row_index++) {
      __m128 packed = pack_float32(current_cols, row_index);
      sums = _mm_add_ps(sums, packed);
    }
    __m128 means = _mm_div_ps(sums, lengths);

    __m128 medians =
        array_percentile_packed_float32(current_cols, 0.5f, values_count);
    __m128 q1s =
        array_percentile_packed_float32(current_cols, 0.25f, values_count);
    __m128 q3s =
        array_percentile_packed_float32(current_cols, 0.75f, values_count);

    __m128 variances = _mm_setzero_ps();
    for (int row_index = 0; row_index < values_count; row_index++) {
      __m128 packed = pack_float32(current_cols, row_index);
      __m128 deviation = _mm_sub_ps(packed, means);
      __m128 sqr_deviation = _mm_mul_ps(deviation, deviation);
      variances = _mm_add_ps(variances, _mm_div_ps(sqr_deviation, lengths));
    }
    __m128 stdevs = _mm_sqrt_ps(variances);

    for (int simd_slot_index = 0; simd_slot_index < simd_pack_size;
         simd_slot_index++) {
      if ((variable_index + simd_slot_index) < array_count) {
        VALUE h_result = rb_hash_new();

        float min = MM_GET_INDEX_FLOAT(mins, simd_slot_index);
        float max = MM_GET_INDEX_FLOAT(maxes, simd_slot_index);
        float mean = MM_GET_INDEX_FLOAT(means, simd_slot_index);
        float median = MM_GET_INDEX_FLOAT(medians, simd_slot_index);
        float q1 = MM_GET_INDEX_FLOAT(q1s, simd_slot_index);
        float q3 = MM_GET_INDEX_FLOAT(q3s, simd_slot_index);
        float stdev = MM_GET_INDEX_FLOAT(stdevs, simd_slot_index);

        rb_hash_aset(h_result, s_min, DBL2NUM(min));
        rb_hash_aset(h_result, s_max, DBL2NUM(max));
        rb_hash_aset(h_result, s_mean, DBL2NUM(mean));
        rb_hash_aset(h_result, s_median, DBL2NUM(median));
        rb_hash_aset(h_result, s_q1, DBL2NUM(q1));
        rb_hash_aset(h_result, s_q3, DBL2NUM(q3));
        rb_hash_aset(h_result, s_standard_deviation, DBL2NUM(stdev));

        rb_ary_push(a_results, h_result);
      }
    }
  }

  return a_results;
}

VALUE simd_enabled(VALUE self) { return Qtrue; }
#endif

inline double ruby_array_percentile(VALUE sorted_arr, double pct, int length)
{
  if (pct == 1.0) return rb_ary_entry(sorted_arr, length - 1);
  double rank = pct * (double)(length - 1);
  int floored_rank = floor(rank);
  double lower = NUM2DBL(rb_ary_entry(sorted_arr, floored_rank));
  double upper = NUM2DBL(rb_ary_entry(sorted_arr, floored_rank + 1));
  return lower + (upper - lower) * (rank - floored_rank);
}

inline double ruby_array_sum(VALUE arr, int length)
{
  double result = 0.0f;
  for (int i = 0; i < length; i++) {
    double value = RFLOAT_VALUE(rb_ary_entry(arr, i));
    result += value;
  }
  return result;
}

inline double ruby_array_standard_deviation(VALUE arr, double mean, int length)
{
  double variance = 0.0f;
  for (int i = 0; i < length; i++) {
    double value = NUM2DBL(rb_ary_entry(arr, i));
    double deviation = value - mean;
    double sqr_deviation = deviation * deviation;
    variance += (sqr_deviation / (double)length);
  }
  double result = sqrt(variance);
  return result;
}

VALUE descriptive_statistics(VALUE self, VALUE arrays)
{
  int cols = rb_array_len(arrays);
  int rows = rb_array_len(rb_ary_entry(arrays, 0));

  VALUE a_results = rb_ary_new();
  VALUE s_min = ID2SYM(rb_intern("min"));
  VALUE s_max = ID2SYM(rb_intern("max"));
  VALUE s_mean = ID2SYM(rb_intern("mean"));
  VALUE s_median = ID2SYM(rb_intern("median"));
  VALUE s_q1 = ID2SYM(rb_intern("q1"));
  VALUE s_q3 = ID2SYM(rb_intern("q3"));
  VALUE s_standard_deviation = ID2SYM(rb_intern("standard_deviation"));

  for (int variable_index = 0; variable_index < cols; variable_index++) {
    VALUE col = rb_ary_entry(arrays, variable_index);
    VALUE h_result = rb_hash_new();

    rb_ary_sort_bang(col);

    VALUE min = rb_ary_entry(col, 0);
    VALUE max = rb_ary_entry(col, rows - 1);
    double median = ruby_array_percentile(col, 0.5, rows);
    double q1 = ruby_array_percentile(col, 0.25, rows);
    double q3 = ruby_array_percentile(col, 0.75, rows);
    double sum = ruby_array_sum(col, rows);
    double mean = sum / (double)rows;
    double stdev = ruby_array_standard_deviation(col, mean, rows);

    rb_hash_aset(h_result, s_min, min);
    rb_hash_aset(h_result, s_max, max);
    rb_hash_aset(h_result, s_mean, DBL2NUM(mean));
    rb_hash_aset(h_result, s_median, DBL2NUM(median));
    rb_hash_aset(h_result, s_q1, DBL2NUM(q1));
    rb_hash_aset(h_result, s_q3, DBL2NUM(q3));
    rb_hash_aset(h_result, s_standard_deviation, DBL2NUM(stdev));

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
