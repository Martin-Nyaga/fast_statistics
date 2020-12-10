#include <algorithm>
#include <functional>
#include <iostream>

#include "ruby.h"

#ifdef HAVE_XMMINTRIN_H
#include <xmmintrin.h>
#define MM_GET_INDEX_FLOAT(packed, index) *(((float*)&packed) + index);
#define MM_GET_INDEX_DOUBLE(packed, index) *(((double*)&packed) + index);

#endif

namespace array_2d
{

struct Stats {
  double min;
  double max;
  double mean;
  double median;
  double q1;
  double q3;
  double standard_deviation;

  Stats()
  {
    min = 0.0, max = 0.0, mean = 0.0, median = 0.0, q1 = 0.0, q3 = 0.0, standard_deviation = 0.0;
  };
};

#ifdef HAVE_XMMINTRIN_H
struct Packed32 {
  typedef __m128 PackedSize;
};
struct Packed64 {
  typedef __m128d PackedSize;
};
struct Unpacked {
  typedef double PackedSize;
};
#endif

template<typename T, typename S>
class DFloat
{
  inline T* base_ptr(int col) { return entries + (col * rows); }
  inline void sort(T* col);
  inline double percentile(T* col, T pct);
  inline double sum(T* col);
  inline double standard_deviation(T* col, T mean);
  T safe_entry(int col, int row);
  void sort_columns(int start_col, int pack_size);

#ifdef HAVE_XMMINTRIN_H
  inline typename S::PackedSize percentile_packed(int start_col, float pct);
  inline typename S::PackedSize pack(int start_col, int row);
#endif

public:
  int cols;
  int rows;
  T* entries;
  Stats* stats;

  DFloat<T, S>(int cols, int rows, T* entries);
  DFloat<T, S>(VALUE ruby_arr);
  Stats* descriptive_statistics();

#ifdef HAVE_XMMINTRIN_H
  Stats* descriptive_statistics_simd();
#endif

  ~DFloat()
  {
    std::cout << "Freeing entries" << std::endl;
    free(entries);
    delete[] stats;
  }
};
typedef DFloat<double, Unpacked> DFloat64Unpacked;

template<typename T, typename S>
inline DFloat<T, S>::DFloat(VALUE arrays)
{
  Check_Type(arrays, T_ARRAY);

  cols = rb_array_len(arrays);
  rows = rb_array_len(rb_ary_entry(arrays, 0));
  entries = (T*)malloc(cols * rows * sizeof(T));
  stats = NULL;

  for (int j = 0; j < cols; j++) {
    for (int i = 0; i < rows; i++) {
      entries[j * rows + i] = (T)NUM2DBL(rb_ary_entry(rb_ary_entry(arrays, j), i));
    }
  }
}

template<typename T, typename S>
inline void
DFloat<T, S>::sort(T* col)
{
  std::sort(col, col + rows);
}

template<typename T, typename S>
inline double
DFloat<T, S>::percentile(T* col, T pct)
{
  if (pct == 1.0) return col[rows - 1];
  double rank = pct * (double)(rows - 1);
  int floored_rank = floor(rank);
  double lower = col[floored_rank];
  double upper = col[floored_rank + 1];
  return lower + (upper - lower) * (rank - floored_rank);
}

template<typename T, typename S>
inline double
DFloat<T, S>::sum(T* col)
{
  double sum = 0.0;
  for (int row = 0; row < rows; row++) {
    sum += col[row];
  }
  return sum;
}

template<typename T, typename S>
inline double
DFloat<T, S>::standard_deviation(T* col, T mean)
{
  double variance = 0.0f;
  for (int i = 0; i < rows; i++) {
    T value = col[i];
    double deviation = value - mean;
    double sqr_deviation = deviation * deviation;
    variance += (sqr_deviation / (double)rows);
  }
  double result = sqrt(variance);
  return result;
}

template<typename T, typename S>
inline Stats*
DFloat<T, S>::descriptive_statistics()
{
  stats = new Stats[cols];

  for (int col = 0; col < cols; col++) {
    Stats var_stats;
    T* col_arr = base_ptr(col);

    sort(col_arr);

    var_stats.min = col_arr[0];
    var_stats.max = col_arr[rows - 1];
    var_stats.median = percentile(col_arr, 0.5);
    var_stats.q1 = percentile(col_arr, 0.25);
    var_stats.q3 = percentile(col_arr, 0.75);
    double total = sum(col_arr);
    var_stats.mean = total / (T)rows;
    var_stats.standard_deviation = standard_deviation(col_arr, var_stats.mean);

    stats[col] = var_stats;
  }

  return stats;
}

template<typename T, typename S>
inline T
DFloat<T, S>::safe_entry(int col, int row)
{
  if (col < cols && row < rows) {
    return *(base_ptr(col) + row);
  } else {
    return 0;
  }
}

#ifdef HAVE_XMMINTRIN_H
template<typename T, typename S>
inline void
DFloat<T, S>::sort_columns(int start_col, int pack_size) 
{
    for (int i = 0; i < pack_size; i++) {
      if ((start_col + i) < cols) {
        T* col_arr = base_ptr(start_col + i);
        sort(col_arr);
      }
    }
}
#endif
} // namespace array_2d
