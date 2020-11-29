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

have_header("xmmintrin.h")

create_makefile("fast_statistics/fast_statistics")
