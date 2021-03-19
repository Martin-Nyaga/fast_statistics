#include "array_2d.h"
#include "debug.h"

namespace array_2d
{

DFloat::~DFloat()
{
  free(entries);
  delete[] stats;
}

DFloat::DFloat(VALUE arrays)
{
  cols = rb_array_len(arrays);
  rows = rb_array_len(rb_ary_entry(arrays, 0));
  entries = (double*)malloc(cols * rows * sizeof(double));
  stats = NULL;

  for (int j = 0; j < cols; j++) {
    for (int i = 0; i < rows; i++) {
      entries[j * rows + i] = (double)NUM2DBL(rb_ary_entry(rb_ary_entry(arrays, j), i));
    }
  }
}

inline void
DFloat::sort(double* col)
{
  std::sort(col, col + rows);
}

inline double
DFloat::percentile(double* col, double pct)
{
  if (pct == 1.0) return col[rows - 1];
  double rank = pct * (double)(rows - 1);
  int floored_rank = floor(rank);
  double lower = col[floored_rank];
  double upper = col[floored_rank + 1];
  return lower + (upper - lower) * (rank - floored_rank);
}

inline double
DFloat::sum(double* col)
{
  double sum = 0.0;
  for (int row = 0; row < rows; row++) {
    sum += col[row];
  }
  return sum;
}

inline double
DFloat::standard_deviation(double* col, double mean)
{
  double variance = 0.0f;
  for (int i = 0; i < rows; i++) {
    double value = col[i];
    double deviation = value - mean;
    double sqr_deviation = deviation * deviation;
    variance += (sqr_deviation / (double)rows);
  }
  double result = sqrt(variance);
  return result;
}

Stats*
DFloat::descriptive_statistics()
{
  stats = new Stats[cols];

  for (int col = 0; col < cols; col++) {
    Stats var_stats;
    double* col_arr = base_ptr(col);

    sort(col_arr);

    var_stats.min = col_arr[0];
    var_stats.max = col_arr[rows - 1];
    var_stats.median = percentile(col_arr, 0.5);
    var_stats.q1 = percentile(col_arr, 0.25);
    var_stats.q3 = percentile(col_arr, 0.75);
    double total = sum(col_arr);
    var_stats.mean = total / (double)rows;
    var_stats.standard_deviation = standard_deviation(col_arr, var_stats.mean);

    stats[col] = var_stats;
  }

  return stats;
}

#ifdef HAVE_XMMINTRIN_H
inline double
DFloat::safe_entry(int col, int row)
{
  if (col < cols) {
    return *(base_ptr(col) + row);
  } else {
    return 0;
  }
}

inline void
DFloat::sort_columns(int start_col, int pack_size)
{
  for (int i = 0; i < pack_size; i++) {
    if ((start_col + i) < cols) {
      double* col_arr = base_ptr(start_col + i);
      sort(col_arr);
    }
  }
}

inline __m128d
DFloat::pack(int start_col, int row)
{
  __m128d packed = _mm_set_pd(safe_entry(start_col + 1, row), safe_entry(start_col + 0, row));
  return packed;
}

inline __m128d
DFloat::percentile_packed(int start_col, float pct)
{
  if (pct == 1.0) return pack(start_col, rows - 1);
  double rank = pct * (double)(rows - 1);
  int floored_rank = floor(rank);
  __m128d lower = pack(start_col, floored_rank);
  __m128d upper = pack(start_col, floored_rank + 1);
  __m128d upper_minus_lower = _mm_sub_pd(upper, lower);
  __m128d rank_minus_floored_rank = _mm_sub_pd(_mm_set_pd1(rank), _mm_set_pd1((float)floored_rank));
  return _mm_add_pd(lower, _mm_mul_pd(upper_minus_lower, rank_minus_floored_rank));
}

Stats*
DFloat::descriptive_statistics_packed()
{
  stats = new Stats[cols];
  const int simd_pack_size = 2;

  __m128d lengths = _mm_set_pd1((double)rows);
  for (int col = 0; col < cols; col += simd_pack_size) {
    sort_columns(col, simd_pack_size);

    __m128d mins = pack(col, 0);
    __m128d maxes = pack(col, rows - 1);
    __m128d sums = _mm_setzero_pd();
    for (int row_index = 0; row_index < rows; row_index++) {
      __m128d packed = pack(col, row_index);
      sums = _mm_add_pd(sums, packed);
    }
    __m128d means = _mm_div_pd(sums, lengths);

    __m128d medians = percentile_packed(col, 0.5f);
    __m128d q1s = percentile_packed(col, 0.25f);
    __m128d q3s = percentile_packed(col, 0.75f);

    __m128d variances = _mm_setzero_pd();
    for (int row_index = 0; row_index < rows; row_index++) {
      __m128d packed = pack(col, row_index);
      __m128d deviation = _mm_sub_pd(packed, means);
      __m128d sqr_deviation = _mm_mul_pd(deviation, deviation);
      variances = _mm_add_pd(variances, _mm_div_pd(sqr_deviation, lengths));
    }
    __m128d stdevs = _mm_sqrt_pd(variances);

    for (int simd_slot_index = 0; simd_slot_index < simd_pack_size; simd_slot_index++) {
      if ((col + simd_slot_index) < cols) {
        Stats var_stats;
        var_stats.min = MM_GET_INDEX(mins, simd_slot_index);
        var_stats.max = MM_GET_INDEX(maxes, simd_slot_index);
        var_stats.mean = MM_GET_INDEX(means, simd_slot_index);
        var_stats.median = MM_GET_INDEX(medians, simd_slot_index);
        var_stats.q1 = MM_GET_INDEX(q1s, simd_slot_index);
        var_stats.q3 = MM_GET_INDEX(q3s, simd_slot_index);
        var_stats.standard_deviation = MM_GET_INDEX(stdevs, simd_slot_index);

        stats[col + simd_slot_index] = var_stats;
      }
    }
  }

  return stats;
}

#endif
} // namespace array_2d
