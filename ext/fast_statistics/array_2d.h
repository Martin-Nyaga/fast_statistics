#ifndef ARRAY_2D_H
#define ARRAY_2D_H
#include <algorithm>
#include "ruby.h"

#ifdef HAVE_XMMINTRIN_H
#include <xmmintrin.h>
#define MM_GET_INDEX_FLOAT(packed, index) *(((floadouble*)&packed) + index);
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

class DFloat
{
  inline double* base_ptr(int col) { return entries + (col * rows); }
  inline void sort(double* col);
  inline double percentile(double* col, double pct);
  inline double sum(double* col);
  inline double standard_deviation(double* col, double mean);

#ifdef HAVE_XMMINTRIN_H
  inline double safe_entry(int col, int row);
  inline void sort_columns(int start_col, int pack_size);
  inline __m128d percentile_packed(int start_col, float pct);
  inline __m128d pack(int start_col, int row);
#endif

public:
  int cols;
  int rows;
  double* entries;
  Stats* stats;

  DFloat(VALUE ruby_arr);
  ~DFloat();

  Stats* descriptive_statistics();

#ifdef HAVE_XMMINTRIN_H
  Stats* descriptive_statistics_packed();
#endif
};
} // namespace array_2d
#endif
