require "bundler/gem_tasks"
require "rake/extensiontask"

task :default => :spec

# Compile
Rake::ExtensionTask.new "fast_statistics" do |ext|
  ext.lib_dir = "lib/fast_statistics"
end

# Rspec
begin
  require 'rspec/core/rake_task'
  RSpec::Core::RakeTask.new(:spec, [:spec] => [:compile])
rescue LoadError
end

# Benchmark
task :benchmark => :compile do 
  require_relative "./benchmark/bench"
  bench = FastStatistics::Benchmark.new
  bench.compare_results!
  bench.benchmark!
end
