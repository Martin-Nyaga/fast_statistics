#ifdef HAVE_XMMINTRIN_H
#include "packed.h"
#include "array_2d.h"

namespace array_2d {
namespace packed {

/*
 * Packed descriptive statistics
 *
 * def descriptive_statistics
 */
extern VALUE descriptive_statistics(VALUE self) {
  GET_DFLOAT(self, dfloat);
  Stats *stats = dfloat->descriptive_statistics_packed();
  return ::array_2d::build_results_hashes(stats, dfloat->cols);
}

} // namespace packed
} // namespace array_2d
#endif
