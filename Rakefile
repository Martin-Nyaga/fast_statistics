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
  RSpec::Core::RakeTask.new(:spec, [:spec] => [:clean, :compile])
rescue LoadError
end

# Benchmark
task :benchmark => [:clean, :compile] do 
  require_relative "./benchmark/bench"
  bench = FastStatistics::Benchmark.new(subject: MeanBenchmark.new)
  bench.compare_results!
  bench.benchmark_ips!
end

task :profile => [:clean, :compile] do
  require "fast_statistics"
  $stdout.sync = true

  variables = 12
  length = 100_000
  data = (0..(variables - 1)).map { (0..(length - 1)).map { rand } }
  FastStatistics::Array2D.new(data, dtype: :float).mean.to_a
  puts
  puts
end
