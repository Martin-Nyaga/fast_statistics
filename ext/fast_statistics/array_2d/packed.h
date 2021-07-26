#ifndef PACKED_H
#define PACKED_H

#include <ruby.h>

#include "dfloat.h"
#include "utils.h"

namespace array_2d
{

extern const rb_data_type_t dfloat_wrapper;

namespace packed
{

VALUE
descriptive_statistics(VALUE self);

}
}

#endif
