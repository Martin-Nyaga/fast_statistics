require "mkmf"

# Enable debug compile using DEBUG env var
if ENV["DEBUG"]
  puts "Compiling in debug mode..."
  CONFIG["debugflags"] = "-g"
  CONFIG["optflags"] = "-O0"
  $defs << "-DDEBUG"
end

# Compile with C++11
$CXXFLAGS += " -std=c++11 "

extension_name = "fast_statistics/fast_statistics"

dir_config(extension_name)

$srcs = Dir.glob("#{$srcdir}/**/*.cpp").map { |path| File.basename(path) }
Dir.glob("#{$srcdir}/*/") do |path|
  dir =  File.basename(path)
  $VPATH << "$(srcdir)/#{dir}"
  $INCFLAGS << " -I$(srcdir)/#{dir}"
end

have_header("xmmintrin.h")
create_makefile("fast_statistics/fast_statistics")
