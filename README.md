# FastStatistics
![Build Status](https://travis-ci.com/Martin-Nyaga/fast_statistics.svg?branch=master)

A high performance native ruby extension (written in C++) for computation of
descriptive statistics.

## Overview
This gem provides fast computation of descriptive statistics (min, max, mean,
median, 1st and 3rd quartiles, population standard deviation) for a
multivariate dataset (represented as a 2D array) in ruby.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'fast_statistics'
```

And then execute:

    $ bundle install

Or install it yourself as:

    $ gem install fast_statistics

## Usage

Given you have some multivariate (2-dimensional) data:
```ruby
data = [
  [0.6269, 0.3783, 0.1477, 0.2374],
  [0.4209, 0.1055, 0.8000, 0.2023],
  [0.1124, 0.1021, 0.1936, 0.8566],
  [0.6454, 0.5362, 0.4567, 0.8309],
  [0.4828, 0.1572, 0.5706, 0.4085],
  [0.5594, 0.0979, 0.4078, 0.5885],
  [0.8659, 0.5346, 0.5566, 0.6166],
  [0.7256, 0.5841, 0.8546, 0.3918]
]
```

You can compute descriptive statistics for all the inner arrays as follows:

```ruby
require "fast_statistics"

FastStatistics::Array2D.new(data).descriptive_statistics
# Result: 
#
# [{:min=>0.1477,
#   :max=>0.6269,
#   :mean=>0.347575,
#   :median=>0.30785,
#   :q1=>0.214975,
#   :q3=>0.44045,
#   :standard_deviation=>0.18100761551658537},
#  {:min=>0.1055,
#   :max=>0.8,
#   :mean=>0.38217500000000004,
#   :median=>0.3116,
#   :q1=>0.1781,
#   :q3=>0.515675,
#   :standard_deviation=>0.26691825878909076},
#  ...,
#  {:min=>0.3918,
#   :max=>0.8546,
#   :mean=>0.639025,
#   :median=>0.6548499999999999,
#   :q1=>0.536025,
#   :q3=>0.75785,
#   :standard_deviation=>0.1718318709523935}]
```

## Implementation Notes

The following factors combined help this gem achieve very high performance
compared to available alternatives and up to 15x speed compared to hand-written
computations in ruby:

- It is written in C++ and so can leverage the speed of native execution. 
- It minimises the number of operations by calculating the statistics in as few
  operations as possible (1 sort + 2 loops). Most native alternatives don't
  provide a built in way to get all these statistics at once, and so typically
  require looping through the data more times than is necessary.
- This gem uses explicit 128-bit-wide SIMD intrinsics (on platforms where they
  are available) to parallelize computations for 2 variables at the
  same time where possible giving an additional speed advantage.

## Benchmarks

Some alternatives compared are:
- [descriptive_statistics](https://github.com/thirtysixthspan/descriptive_statistics)
- [ruby-native-statistics](https://github.com/corybuecker/ruby-native-statistics)
- [Numo::NArray](https://github.com/ruby-numo/numo-narray)
- Hand-written ruby (using the same algorithm implemented in C++ in this gem)

You can reivew the benchmark implementations at `benchmark/benchmark.rb` and run the
benchmark with `rake benchmark`. 

Results:
```
Comparing calculated statistics with 10 values for 8 variables...
Test passed, results are equal to 6 decimal places!

Benchmarking with 100,000 values for 12 variables...
Warming up --------------------------------------
descriptive_statistics   1.000  i/100ms
           Custom ruby   1.000  i/100ms
                narray   1.000  i/100ms
ruby_native_statistics   1.000  i/100ms
        FastStatistics   3.000  i/100ms
Calculating -------------------------------------
descriptive_statistics   0.473  (± 0.0%) i/s -      3.000  in   6.354555s
           Custom ruby   2.518  (± 0.0%) i/s -     13.000  in   5.169084s
                narray   4.231  (± 0.0%) i/s -     22.000  in   5.210299s
ruby_native_statistics   5.962  (± 0.0%) i/s -     30.000  in   5.041869s
        FastStatistics   28.417  (±10.6%) i/s -    141.000  in   5.012229s

Comparison:
        FastStatistics:   28.4 i/s
ruby_native_statistics:    6.0 i/s - 4.77x  (± 0.00) slower
                narray:    4.2 i/s - 6.72x  (± 0.00) slower
           Custom ruby:    2.5 i/s - 11.29x  (± 0.00) slower
descriptive_statistics:    0.5 i/s - 60.09x  (± 0.00) slower
```

## Contributing

Bug reports and pull requests are welcome on GitHub at
https://github.com/Martin-Nyaga/fast_statistics.

## License

The gem is available as open source under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
