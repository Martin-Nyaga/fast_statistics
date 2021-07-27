#ifndef ARRAY_2D_H
#define ARRAY_2D_H

#include <ruby.h>

#include "array_2d/packed.h"
#include "array_2d/unpacked.h"
#include "dfloat.h"
#include "utils.h"

#define GET_DFLOAT_UNTYPED(obj, var)                                                               \
  void *var;                                                                                       \
  TypedData_Get_Struct((obj), void *, &dfloat_wrapper, (var));

#define GET_DFLOAT(obj, var)                                                                       \
  GET_DFLOAT_UNTYPED(obj, var##_untyped);                                                          \
  DFloat *var = ((DFloat *)var##_untyped);

namespace array_2d {

void setup(VALUE m_fast_statistics);
VALUE build_results_hashes(Stats *stats, int num_variables);

} // namespace array_2d

#endif
