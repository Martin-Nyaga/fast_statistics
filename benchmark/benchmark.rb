# frozen_string_literal: true

require "bundler/setup"

require_relative "./base"
require_relative "./helpers"

require "fast_statistics"
require "descriptive_statistics/safe"
require "ruby_native_statistics"
require "numo/narray"

class DescriptiveStatsBenchmark < BaseBenchmark
  class << self
    include Helpers
  end

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
