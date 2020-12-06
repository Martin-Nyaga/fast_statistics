# frozen_string_literal: true

require "bundler/setup"

require "fast_statistics"
require "benchmark"
require "benchmark/ips"
require "terminal-table"
require "descriptive_statistics/safe"
require "ruby_native_statistics"
require "numo/narray"

# Custom Ruby Statisitics implementation for comparison
module RubyStatistics
  def self.descriptive_statistics(data)
    data.map do |arr|
      arr.sort!

      min = arr.first
      max = arr.last
      length = arr.length
      median = self.percentile(50, arr, length)
      q1 = self.percentile(25, arr, length)
      q3 = self.percentile(75, arr, length)
      sum = arr.inject(0) { |sum, x| sum + x}

      mean = sum / length
      variance = arr.inject(0) { |var, x| var += ((x - mean) ** 2) / length }
      standard_deviation = Math.sqrt(variance)
      {
        mean: mean,
        min: min,
        max: max,
        median: median,
        q1: q1,
        q3: q3,
        standard_deviation: standard_deviation
      }
    end
  end

  def self.descriptive_statistics_narray(data)
    data.map do |arr|
      narr = Numo::DFloat[arr]
      narr.sort
      min = narr[0]
      length = arr.length
      max = narr[length - 1]
      median = self.percentile(50, narr, length)
      q1 = self.percentile(25, narr, length)
      q3 = self.percentile(75, narr, length)
      sum = narr.sum
      mean = sum / length
      variance = 0
      narr.each { |x| variance += ((x - mean) ** 2) / length }
      standard_deviation = Math.sqrt(variance)
      {
        mean: mean,
        min: min,
        max: max,
        median: median,
        q1: q1,
        q3: q3,
        standard_deviation: standard_deviation
      }
    end
  end

  def self.percentile(p, arr, len)
    return arr[len - 1] if p == 100
    rank = p / 100.0 * (len - 1)
    lower = arr[rank.floor]
    upper = arr[rank.floor + 1]
    lower + (upper - lower) * (rank - rank.floor)
  end
end

module FastStatistics
  class Benchmark
    def tests
      [
        ["Ruby (desc_stats)", :test_ruby_descriptive_statistics],
        ["Ruby (custom)", :test_ruby],
        ["Ruby (narray)", :test_ruby_narray],
        ["Ruby (native_stats)", :test_ruby_native_statistics],
        ["Fast (unpacked)", :test_native],
        ["Fast (float32)", :test_native_float32],
        ["Fast (float64)", :test_native_float64],
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

      ::Benchmark.bmbm do |x|
        tests.each do |(name, method)|
          x.report(name) do
            send(method, data)
          end
        end
      end
    end

    def benchmark_ips!(benchmark_count = 100_000, variables_count = 12)
      data = generate_data(benchmark_count, variables_count)
      puts("Benchmarking with #{format_number(benchmark_count)} values for #{data.length} variables...")

      ::Benchmark.ips do |x|
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

    def test_ruby_narray(data)
      RubyStatistics.descriptive_statistics_narray(data)
    end

    def test_ruby_descriptive_statistics(data)
      data.map do |arr|
        stats = DescriptiveStatistics::Stats.new(arr)
        {
          mean: stats.mean,
          min: stats.min,
          max: stats.max,
          median: stats.median,
          q1: stats.percentile(25),
          q3: stats.percentile(75),
          standard_deviation: stats.standard_deviation
        }
      end
    end

    def test_ruby_native_statistics(data)
      data.map do |arr|
        {
          mean: arr.mean,
          min: arr.min,
          max: arr.max,
          median: arr.median,
          q1: arr.percentile(0.25),
          q3: arr.percentile(0.75),
          standard_deviation: arr.stdevp
        }
      end
    end

    def test_native(data)
      FastStatistics.descriptive_statistics_unpacked(data)
    end

    def test_native_float32(data)
      FastStatistics.descriptive_statistics_packed_float32(data)
    end

    def test_native_float64(data)
      FastStatistics.descriptive_statistics_packed_float64(data)
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
      values.combination(2).each do |expected, actual|
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
