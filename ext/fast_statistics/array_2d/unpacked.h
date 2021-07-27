#ifndef UNPACKED_H
#define UNPACKED_H

#include <ruby.h>

#include "dfloat.h"

namespace array_2d {

extern const rb_data_type_t dfloat_wrapper;

namespace unpacked {

VALUE
descriptive_statistics(VALUE self);

} // namespace unpacked
} // namespace array_2d

#endif
