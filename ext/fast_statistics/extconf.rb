require "mkmf"

# Enable debug compile using DEBUG env var
if ENV["DEBUG"]
  puts "Compiling in debug mode..."
  CONFIG["debugflags"] = "-g"
  CONFIG["optflags"] = "-O0"
  $defs << "-DDEBUG"
end

# Disable warnings
[
  / -Wdeclaration-after-statement/,
  / -Wimplicit-int/,
  / -Wimplicit-function-declaration/,
].each do |flag|
  CONFIG["warnflags"].slice!(flag)
end

have_header("xmmintrin.h")
create_makefile("fast_statistics/fast_statistics")
