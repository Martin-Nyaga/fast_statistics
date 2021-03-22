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

class BaseBenchmark
  class << self
    include Helpers

    @@tests = []

    def benchmark(name, &block)
      @@tests.push(TestCase.new(name, block))
    end

    def tests
      @@tests
    end

    TestCase = Struct.new(:name, :block) do
      def run(data)
        block.call(data)
      end
    end
  end
end

class DescriptiveStatsBenchmark < BaseBenchmark
  benchmark "descriptive_statistics" do |data|
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

  benchmark "Custom ruby" do |data|
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

  benchmark "narray" do |data|
    data.map do |arr|
      narr = Numo::DFloat[arr]
      narr.sort
      min = narr[0]
      length = arr.length
      max = narr[length - 1]
      median = percentile(50, narr, length)
      q1 = percentile(25, narr, length)
      q3 = percentile(75, narr, length)
      mean = narr.mean
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

  benchmark "ruby_native_statistics" do |data|
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

  benchmark "FastStatistics" do |data|
    FastStatistics::Array2D.new(data).descriptive_statistics
  end
end

module FastStatistics
  class Benchmark
    attr_reader :subject
    def initialize(subject:)
      @subject = subject
    end

    def compare_results!(data_points: 10, precision: 6)
      data = generate_data(data_points)
      puts("Comparing calculated statistics with #{format_number(data_points)} values for #{data.length} variables...")

      test_results = subject.tests.map { |test| test.run(data) }

      # Uncomment to print results
      # test_results.zip(subject.tests) do |results, test|
      #   print_results(test.name, results, precision)
      # end

      if assert_values_within_delta(test_results, 10 ** -precision)
        puts("Test passed, results are equal to #{precision} decimal places!")
        puts
      end

    rescue TestFailure => e
      puts("Test results did not match!")
      exit(1)
    end

    def benchmark!(data_points: 100_000, variables_count: 12)
      data = generate_data(data_points, variables_count)
      puts("Benchmarking with #{format_number(data_points)} values for #{data.length} variables...")

      ::Benchmark.bmbm do |x|
        subject.tests.each do |test|
          x.report(test.name) do
            test.run(data)
          end
        end
      end
    end

    def benchmark_ips!(data_points: 100_000, variables_count: 12)
      data = generate_data(data_points, variables_count)
      puts("Benchmarking with #{format_number(data_points)} values for #{data.length} variables...")

      ::Benchmark.ips do |x|
        subject.tests.each do |test|
          x.report(test.name) do
            test.run(data)
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
