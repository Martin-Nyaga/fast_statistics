#include "unpacked.h"
#include "array_2d.h"

namespace array_2d
{
namespace unpacked
{

/*
 * Unpacked descriptive statistics
 *
 * def descriptive_statistics
 */
VALUE
descriptive_statistics(VALUE self)
{
  GET_DFLOAT(self, dfloat);
  Stats* stats = dfloat->descriptive_statistics();
  return build_results_hashes(stats, dfloat->cols);
}

}
}