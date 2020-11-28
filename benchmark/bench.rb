require "bundler/setup"
p
require "fast_statistics"
require "benchmark/ips"
require "terminal-table"
require "descriptive_statistics"
require "ruby_native_statistics"

module RubyStatistics
  def self.descriptive_statistics(data)
    data.map do |arr|
      min = Float::INFINITY
      max = -Float::INFINITY
      sum = 0

      arr.each do |x|
        min = x if x < min
        max = x if x > max
        sum += x
      end

      length = arr.length
      mean = sum / length
      variance = arr.inject(0) { |var, x| var += ((x - mean) ** 2) / length }
      standard_deviation = Math.sqrt(variance)
      {mean: mean, min: min, max: max, variance: variance, standard_deviation: standard_deviation}
    end
  end
end

def test_native(data)
  FastStatistics.descriptive_statistics_unpacked(data)
end

def test_native_simd(data)
  FastStatistics.descriptive_statistics(data)
end

def test_ruby_custom(data)
  RubyStatistics.descriptive_statistics(data)
end

def test_ruby_descriptive_statistics(data)
  data.map do |arr|
    {mean: arr.mean, min: arr.min, max: arr.max, variance: arr.variance, standard_deviation: arr.standard_deviation}
  end
end

def generate_data(length)
  data = (0..3).map { (0..(length - 1)).map { rand } }
end

def print_results(title, results, precision)
  headers = results[0].keys
  values = results.map { |r| r.values.map { |v| "%.#{precision}f" % v } }
  table = Terminal::Table.new(:headings => headers, :rows => values)
  puts(title + ":")
  puts(table)
end

class TestFailure < StandardError
end

def assert_values_within_delta(values, delta)
  values.each_cons(2) do |expected, actual|
    expected.each_with_index do |result, i|
      actual_result = actual[i]

      result.each do |k, v|
        raise TestFailure.new("Results don't match!") unless (actual_result[k] - result[k]).abs < delta
      end
    end
  end

  true
end

def format_number(number)
  number.to_s.reverse.gsub(/(\d{3})(?=\d)/, "\\1,").reverse
end

def compare(comparison_count = 10, precision = 6)
  puts("Comparing calculated statistics with #{format_number(comparison_count)} values...")

  data = generate_data(comparison_count)
  ruby_custom_results = test_ruby_custom(data)
  ruby_desc_results = test_ruby_descriptive_statistics(data)
  native_results = test_native(data)
  native_simd_results = test_native_simd(data)

  print_results("Ruby (Custom)", ruby_custom_results, precision)
  print_results("Ruby (Desc Stats)", ruby_desc_results, precision)
  print_results("FastStatistics", native_results.to_a, precision)
  print_results("FastStatistics (Simd)", native_simd_results.to_a, precision)

  results = [
    ruby_custom_results,
    ruby_desc_results,
    native_results,
    native_simd_results
  ]

  if assert_values_within_delta(results, 10 ** (-precision))
    puts("Test passed, results are equal to #{precision} decimal places!")
    puts
  end

rescue TestFailure => e
  puts("Test results did not match!")
  exit(1)
end

def benchmark(benchmark_count = 100_000)
  puts("Benchmarking with #{format_number(benchmark_count)} values...")
  data = generate_data(benchmark_count)

  Benchmark.ips do |x|
    x.config(warmup: 5)

    x.report("Ruby (Desc Stats)") do
      test_ruby_descriptive_statistics(data)
    end

    x.report("Ruby (Custom)") do
      test_ruby_custom(data)
    end

    x.report("FastStatistics") do
      test_native(data)
    end

    x.report("FastStatistics (Simd)") do
      test_native_simd(data)
    end

    x.compare!
  end
end

compare
benchmark
