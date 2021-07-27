require "mkmf"

class MkmfConfig
  attr_reader :extension_name, :debug, :disable_instrinsics

  def initialize(extension_name:, debug:, disable_instrinsics:)
    @extension_name = extension_name
    @debug = debug
    @disable_instrinsics = disable_instrinsics
  end

  # Enable debug compile using DEBUG env var
  def configure_debugging
    if debug
      puts "Compiling in debug mode..."
      CONFIG["debugflags"] = "-g"
      CONFIG["optflags"] = "-O0"
      $defs << "-DDEBUG"
    end
  end

  def configure_cpp_standard
    $CXXFLAGS += " -std=c++11 "
  end

  def configure_compilation_files
    dir_config(extension_name)

    $srcs = Dir.glob("#{$srcdir}/**/*.cpp").map { |path| File.basename(path) }
    Dir.glob("#{$srcdir}/*/") do |path|
      dir =  File.basename(path)
      $VPATH << "$(srcdir)/#{dir}"
      $INCFLAGS << " -I$(srcdir)/#{dir}"
    end
  end

  def configure_headers
    return if disable_instrinsics
    have_header("xmmintrin.h")
  end

  def configure_makefile
    create_makefile(extension_name)
  end

  def configure
    configure_debugging
    configure_cpp_standard
    configure_compilation_files

    configure_headers
    configure_makefile
  end
end

MkmfConfig.new(
  extension_name: "fast_statistics/fast_statistics",
  debug: ENV["DEBUG"],
  disable_instrinsics: ENV["DISABLE_INTRINSICS"]
).configure
