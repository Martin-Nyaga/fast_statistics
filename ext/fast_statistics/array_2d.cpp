#include "array_2d.h"

namespace array_2d
{

#ifdef HAVE_XMMINTRIN_H
template<>
inline Packed32::PackedSize
DFloat<float, Packed32>::pack(int start_col, int row)
{
  __m128 packed = _mm_set_ps(
    safe_entry(start_col + 3, row),
    safe_entry(start_col + 2, row),
    safe_entry(start_col + 1, row),
    safe_entry(start_col + 0, row));
  return packed;
}

template<>
inline Packed64::PackedSize
DFloat<double, Packed64>::pack(int start_col, int row)
{
  __m128d packed = _mm_set_pd(safe_entry(start_col + 1, row), safe_entry(start_col + 0, row));
  return packed;
}

template<>
inline __m128
DFloat<float, Packed32>::percentile_packed(int start_col, float pct)
{
  if (pct == 1.0) return pack(start_col, rows - 1);
  float rank = pct * (float)(rows - 1);
  int floored_rank = floor(rank);
  __m128 lower = pack(start_col, floored_rank);
  __m128 upper = pack(start_col, floored_rank + 1);
  __m128 upper_minus_lower = _mm_sub_ps(upper, lower);
  __m128 rank_minus_floored_rank = _mm_sub_ps(_mm_set_ps1(rank), _mm_set_ps1((float)floored_rank));
  return _mm_add_ps(lower, _mm_mul_ps(upper_minus_lower, rank_minus_floored_rank));
}

template<>
inline Packed64::PackedSize
DFloat<double, Packed64>::percentile_packed(int start_col, float pct)
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

template<>
Stats*
DFloat<float, Packed32>::descriptive_statistics_simd()
{
  stats = new Stats[cols];
  const int simd_pack_size = 4;

  __m128 lengths = _mm_set_ps1((float)rows);
  for (int col = 0; col < cols; col += simd_pack_size) {
    sort_columns(col, simd_pack_size);

    __m128 mins = pack(col, 0);
    __m128 maxes = pack(col, rows - 1);
    __m128 sums = _mm_setzero_ps();
    for (int row_index = 0; row_index < rows; row_index++) {
      __m128 packed = pack(col, row_index);
      sums = _mm_add_ps(sums, packed);
    }
    __m128 means = _mm_div_ps(sums, lengths);

    __m128 medians = percentile_packed(col, 0.5f);
    __m128 q1s = percentile_packed(col, 0.25f);
    __m128 q3s = percentile_packed(col, 0.75f);

    __m128 variances = _mm_setzero_ps();
    for (int row_index = 0; row_index < rows; row_index++) {
      __m128 packed = pack(col, row_index);
      __m128 deviation = _mm_sub_ps(packed, means);
      __m128 sqr_deviation = _mm_mul_ps(deviation, deviation);
      variances = _mm_add_ps(variances, _mm_div_ps(sqr_deviation, lengths));
    }
    __m128 stdevs = _mm_sqrt_ps(variances);

    for (int simd_slot_index = 0; simd_slot_index < simd_pack_size; simd_slot_index++) {
      if ((col + simd_slot_index) < cols) {
        Stats var_stats;
        var_stats.min = MM_GET_INDEX_FLOAT(mins, simd_slot_index);
        var_stats.max = MM_GET_INDEX_FLOAT(maxes, simd_slot_index);
        var_stats.mean = MM_GET_INDEX_FLOAT(means, simd_slot_index);
        var_stats.median = MM_GET_INDEX_FLOAT(medians, simd_slot_index);
        var_stats.q1 = MM_GET_INDEX_FLOAT(q1s, simd_slot_index);
        var_stats.q3 = MM_GET_INDEX_FLOAT(q3s, simd_slot_index);
        var_stats.standard_deviation = MM_GET_INDEX_FLOAT(stdevs, simd_slot_index);

        stats[col + simd_slot_index] = var_stats;
      }
    }
  }

  return stats;
}

template<>
Stats*
DFloat<double, Packed64>::descriptive_statistics_simd()
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
        var_stats.min = MM_GET_INDEX_DOUBLE(mins, simd_slot_index);
        var_stats.max = MM_GET_INDEX_DOUBLE(maxes, simd_slot_index);
        var_stats.mean = MM_GET_INDEX_DOUBLE(means, simd_slot_index);
        var_stats.median = MM_GET_INDEX_DOUBLE(medians, simd_slot_index);
        var_stats.q1 = MM_GET_INDEX_DOUBLE(q1s, simd_slot_index);
        var_stats.q3 = MM_GET_INDEX_DOUBLE(q3s, simd_slot_index);
        var_stats.standard_deviation = MM_GET_INDEX_DOUBLE(stdevs, simd_slot_index);

        stats[col + simd_slot_index] = var_stats;
      }
    }
  }

  return stats;
}

#endif

} // namespace array_2d
