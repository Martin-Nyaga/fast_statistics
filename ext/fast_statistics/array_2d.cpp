#include "array_2d.h"

namespace array_2d
{

static VALUE cArray2D;

void
free_wrapped_array(void* dfloat)
{
  ((DFloat*)dfloat)->~DFloat();
  free(dfloat);
}

size_t
wrapped_array_size(const void* data)
{
  return sizeof(DFloat);
}

const rb_data_type_t dfloat_wrapper = [] {
  rb_data_type_t wrapper;
  wrapper.wrap_struct_name = "dfloat";
  wrapper.function = { .dmark = NULL, .dfree = free_wrapped_array, .dsize = wrapped_array_size };
  wrapper.data = NULL;
  wrapper.flags = RUBY_TYPED_FREE_IMMEDIATELY;
  return wrapper;
}();

VALUE
rb_alloc(VALUE self)
{
  void* dfloat = (void*)malloc(sizeof(DFloat));
  return TypedData_Wrap_Struct(self, &dfloat_wrapper, dfloat);
}

/*
 * def initialize(arrays)
 */
VALUE
rb_initialize(VALUE self, VALUE arrays)
{
  // Get untyped memory for a dfloat
  GET_DFLOAT_UNTYPED(self, dfloat);

  // Type-check 2D array
  if (TYPE(arrays) != T_ARRAY) {
    new (dfloat) DFloat(arrays, false);
    Check_Type(arrays, T_ARRAY);
    return false;
  }

  if (TYPE(rb_ary_entry(arrays, 0)) != T_ARRAY) {
    new (dfloat) DFloat(arrays, false);
    Check_Type(rb_ary_entry(arrays, 0), T_ARRAY);
    return false;
  }

  new (dfloat) DFloat(arrays, true);
  return self;
}

void
initialize(VALUE mFastStatistics)
{
  cArray2D = rb_define_class_under(mFastStatistics, "Array2D", rb_cData);
  rb_define_alloc_func(cArray2D, rb_alloc);
  rb_define_method(cArray2D, "initialize", RUBY_METHOD_FUNC(rb_initialize), 1);

#ifdef HAVE_XMMINTRIN_H
  rb_define_method(
    cArray2D, "descriptive_statistics", RUBY_METHOD_FUNC(packed::descriptive_statistics), 0);
#else
  rb_define_method(
    cArray2D, "descriptive_statistics", RUBY_METHOD_FUNC(unpacked::descriptive_statistics), 0);
#endif
}

}
