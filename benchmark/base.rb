# frozen_string_literal: true

require "benchmark"
require "benchmark/ips"
require "terminal-table"

class BaseBenchmark
  class << self
    @@tests = []

    def benchmark(name, &block)
      @@tests.push(TestCase.new(name, block))
    end

    def tests
      @@tests
    end

    def compare_results!(data_points: 10, precision: 6)
      data = generate_data(data_points)
      puts("Comparing calculated statistics with #{format_number(data_points)} values for #{data.length} variables...")

      test_results = tests.map { |test| test.run(data) }

      # Uncomment to print results
      # test_results.zip(tests) do |results, test|
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
        tests.each do |test|
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
        tests.each do |test|
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

    TestCase = Struct.new(:name, :block) do
      def run(data)
        block.call(data)
      end
    end

    class TestFailure < StandardError
    end
  end
end
