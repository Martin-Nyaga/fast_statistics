require_relative 'lib/fast_statistics/version'

Gem::Specification.new do |spec|
  spec.name          = "fast_statistics"
  spec.version       = FastStatistics::VERSION
  spec.authors       = ["Martin Nyaga"]
  spec.email         = ["martin@martinnyaga.com"]

  spec.summary       = %q{Fast computation of descriptive statistics in ruby}
  spec.description   = %q{Fast computation of descriptive statistics in ruby using native code and SIMD}
  spec.homepage      = "https://github.com/martin-nyaga/ruby-ffi-simd"
  spec.license       = "MIT"
  spec.required_ruby_version = Gem::Requirement.new(">= 2.3.0")

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = "https://github.com/martin-nyaga/fast_statistics"
  spec.metadata["changelog_uri"] = "https://github.com/martin-nyaga/fast_statistics"

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files         = Dir.chdir(File.expand_path('..', __FILE__)) do
    `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|spec|features)/}) }
  end
  spec.require_paths = ["lib"]

  spec.extensions = %w[ext/fast_statistics/extconf.rb]
end
