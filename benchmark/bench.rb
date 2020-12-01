# frozen_string_literal: true

require "bundler/setup"

require "fast_statistics"
require "benchmark/ips"
require "terminal-table"
require "descriptive_statistics/safe"
require "ruby_native_statistics"

# Custom Ruby Statisitics implementation for comparison
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

module FastStatistics
  class Benchmark
    def tests
      [
        ["Ruby (custom)", :test_ruby],
        ["Ruby (desc_stats)", :test_ruby_descriptive_statistics],
        ["Ruby (native_stats)", :test_ruby_native_statistics],
        ["Fast (unpacked)", :test_native],
        ["Fast (128_float32)", :test_native128_float32],
        ["Fast (128_float64)", :test_native128_float64],
        ["Fast (256_float32)", :test_native256_float32],
        ["Fast (256_float64)", :test_native256_float64],
      ]
    end

    def compare_results!(comparison_count = 10, precision = 6)
      data = generate_data(comparison_count)
      puts("Comparing calculated statistics with #{format_number(comparison_count)} values for #{data.length} variables...")

      test_results = tests.map do |(name, method)|
        results = send(method, data)
        results
      end

      if assert_values_within_delta(test_results, 10 ** -precision)
        puts("Test passed, results are equal to #{precision} decimal places!")
        puts
      end

    rescue TestFailure => e
      puts("Test results did not match!")
      exit(1)
    end

    def benchmark!(benchmark_count = 100_000, variables_count = 12)
      data = generate_data(benchmark_count, variables_count)
      puts("Benchmarking with #{format_number(benchmark_count)} values for #{data.length} variables...")

      ::Benchmark.ips do |x|
        x.config(warmup: 5)

        tests.each do |(name, method)|
          x.report(name) do
            send(method, data)
          end
        end

        x.compare!
      end
    end

    private

    def test_ruby(data)
      RubyStatistics.descriptive_statistics(data)
    end

    def test_ruby_descriptive_statistics(data)
      data.map do |arr|
        stats = DescriptiveStatistics::Stats.new(arr)
        {mean: stats.mean, min: stats.min, max: stats.max, variance: stats.variance, standard_deviation: stats.standard_deviation}
      end
    end

    def test_ruby_native_statistics(data)
      data.map do |arr|
        {mean: arr.mean, min: arr.min, max: arr.max, variance: arr.varp, standard_deviation: arr.stdevp}
      end
    end

    def test_native(data)
      FastStatistics.descriptive_statistics_unpacked(data)
    end

    def test_native128_float32(data)
      FastStatistics.descriptive_statistics_packed128_float32(data)
    end

    def test_native128_float64(data)
      FastStatistics.descriptive_statistics_packed128_float64(data)
    end

    def test_native256_float32(data)
      FastStatistics.descriptive_statistics_packed256_float32(data)
    end

    def test_native256_float64(data)
      FastStatistics.descriptive_statistics_packed256_float64(data)
    end

    def generate_data(length, variables = 8)
      data = (0..(variables - 1)).map { (0..(length - 1)).map { rand } }
    end

    def print_results(title, results, precision)
      headers = results[0].keys
      values = results.map { |r| r.values.map { |v| "%.#{precision}f" % v } }
      table = Terminal::Table.new(headings: headers, rows: values)
      puts(title + ":")
      puts(table)
    end

    class TestFailure < StandardError
    end

    def assert_values_within_delta(values, delta)
      values.each_cons(2) do |expected, actual|
        unless expected.length == actual.length
          raise TestFailure, "Results don't match!"
        end

        expected.each_with_index do |result, i|
          actual_result = actual[i]
          result.each do |k, _v|
            unless (actual_result[k] - result[k]).abs < delta
              raise TestFailure, "Results don't match!"
            end
          end
        end
      end

      true
    end

    def format_number(number)
      number.to_s.reverse.gsub(/(\d{3})(?=\d)/, "\\1,").reverse
    end
  end
end
