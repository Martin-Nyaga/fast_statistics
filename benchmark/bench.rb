# frozen_string_literal: true

require "bundler/setup"

require "fast_statistics"
require "benchmark"
require "benchmark/ips"
require "terminal-table"
require "descriptive_statistics/safe"
require "ruby_native_statistics"
require "numo/narray"

# Useful helper methods
module Helpers
  def percentile(p, arr, len)
    return arr[len - 1] if p == 100
    rank = p / 100.0 * (len - 1)
    lower = arr[rank.floor]
    upper = arr[rank.floor + 1]
    lower + (upper - lower) * (rank - rank.floor)
  end
end

class DescriptiveStatsBenchmark
  include Helpers

  def tests
    [
      ["Ruby (desc_stats)", :descriptive_statistics],
      ["Ruby (custom)", :ruby],
      ["Ruby (narray)", :narray],
      ["Ruby (native_stats)", :ruby_native_statistics],
      ["Fast (float32)", :fast_statistics_float32],
      ["Fast (float64)", :fast_statistics_float64],
    ]
  end

  def descriptive_statistics(data)
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

  def ruby(data)
    data.map do |arr|
      arr.sort!

      min = arr.first
      max = arr.last
      length = arr.length
      median = percentile(50, arr, length)
      q1 = percentile(25, arr, length)
      q3 = percentile(75, arr, length)
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

  def narray(data)
    data.map do |arr|
      narr = Numo::DFloat[arr]
      narr.sort
      min = narr[0]
      length = arr.length
      max = narr[length - 1]
      median = percentile(50, narr, length)
      q1 = percentile(25, narr, length)
      q3 = percentile(75, narr, length)
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

  def ruby_native_statistics(data)
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

  def fast_statistics_float32(data)
    FastStatistics::Array2D.new(data, dtype: :float).descriptive_statistics
  end

  def fast_statistics_float64(data)
    FastStatistics::Array2D.new(data, dtype: :double).descriptive_statistics
  end
end

class MeanBenchmark
  def tests
    [
      ["Ruby (desc_stats)", :descriptive_statistics],
      ["Ruby (custom)", :ruby],
      ["Ruby (narray)", :narray],
      ["Ruby (native_stats)", :ruby_native_statistics],
      ["Fast (float32)", :fast_statistics_float32],
      ["Fast (float64)", :fast_statistics_float64],
      ["Fast (unpacked)", :fast_statistics_unpacked],
    ]
  end

  def descriptive_statistics(data)
    data.map do |arr|
      stats = DescriptiveStatistics::Stats.new(arr)
      stats.mean
    end
  end

  def ruby(data)
    data.map do |arr|
      length = arr.length
      sum = arr.inject(0) { |sum, x| sum + x}
      sum / length
    end
  end

  def narray(data)
    data.map do |arr|
      narr = Numo::DFloat[arr]
      length = arr.length
      sum = narr.sum
      sum / length
    end
  end

  def ruby_native_statistics(data)
    data.map(&:mean)
  end

  def fast_statistics_float32(data)
    FastStatistics::Array2D.new(data, dtype: :float).mean
  end

  def fast_statistics_float64(data)
    FastStatistics::Array2D.new(data, dtype: :double).mean
  end

  def fast_statistics_unpacked(data)
    FastStatistics::Array2D.new(data, packed: false).mean
  end
end

module FastStatistics
  class Benchmark
    attr_reader :subject
    def initialize(subject:)
      @subject = subject
    end

    def compare_results!(comparison_count: 10, precision: 6)
      data = generate_data(comparison_count)
      puts("Comparing calculated statistics with #{format_number(comparison_count)} values for #{data.length} variables...")

      test_results = subject.tests.map do |(name, method)|
        subject.send(method, data)
      end

      if assert_values_within_delta(test_results, 10 ** -precision)
        puts("Test passed, results are equal to #{precision} decimal places!")
        puts
      end

    rescue TestFailure => e
      puts("Test results did not match!")
      exit(1)
    end

    def benchmark!(benchmark_count: 100_000, variables_count: 12)
      data = generate_data(benchmark_count, variables_count)
      puts("Benchmarking with #{format_number(benchmark_count)} values for #{data.length} variables...")

      ::Benchmark.bmbm do |x|
        subject.tests.each do |(name, method)|
          x.report(name) do
          subject.send(method, data)
        end
        end
      end
    end

    def benchmark_ips!(benchmark_count: 100_000, variables_count: 12)
      data = generate_data(benchmark_count, variables_count)
      puts("Benchmarking with #{format_number(benchmark_count)} values for #{data.length} variables...")

      ::Benchmark.ips do |x|
        subject.tests.each do |(name, method)|
          x.report(name) do
          subject.send(method, data)
        end
        x.compare!
        end
      end
    end

    private

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

        expected.each_with_index do |expected_result, i|
          actual_result = actual[i]
          if actual_result.is_a?(Hash) && expected_result.is_a?(Hash)
            expected_result.each do |k, _v|
              assert_in_delta(actual_result[k], expected_result[k], delta)
            end
          else
            assert_in_delta(actual_result, expected_result, delta)
          end
        end
      end

      true
    end

    def assert_in_delta(expected, actual, delta)
      unless (expected - actual).abs < delta
        raise TestFailure, "Results don't match!"
      end

      true
    end

    def format_number(number)
      number.to_s.reverse.gsub(/(\d{3})(?=\d)/, "\\1,").reverse
    end
  end
end
