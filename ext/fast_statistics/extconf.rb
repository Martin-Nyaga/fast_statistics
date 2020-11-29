require "mkmf"

# Enable debug compile using DEBUG env var
CONFIG["debugflags"] = "-g" if ENV["DEBUG"]

# Disable warnings
[
  / -Wdeclaration-after-statement/,
  / -Wno-self-assign/,
  / -Wno-constant-logical-operand/,
  / -Wno-parentheses-equality/
].each do |flag|
  CONFIG["warnflags"].slice!(flag)
end

has_xmmintrin = have_header("xmmintrin.h")
has_immintrin = have_header("immintrin.h")

if has_immintrin
  append_cflags("-mavx")
end

if has_xmmintrin || has_immintrin
  $defs << "-DHAVE_SIMD_INTRINSICS"
end

create_makefile("fast_statistics/fast_statistics")
